#include "APINetLayer.h"
#include "picojson.h"

namespace json = picojson;

CLASS_SOURCE(APINetLayer)


APINetLayer::APINetLayer()
{
}

APINetLayer::~APINetLayer()
{
}

void APINetLayer::OnBegin() 
{
	Super::OnBegin();

	m_controller = GetGame()->GetFirstObject<OAPIController>();
	if (m_controller == nullptr)
		LOG_ERROR("APINetLayer can only be used if an OAPIController is active");
}

void APINetLayer::OnEncodeHandshake(const NetIdentity& host, ByteBuffer& outBuffer)
{
	Encode<string>(outBuffer, m_password);

	// Encode login information
	Encode<string>(outBuffer, m_controller->GetUserID());
	Encode<string>(outBuffer, m_controller->GetUserSessionID());
}

NetResponseCode APINetLayer::OnDecodeHandshake(const NetIdentity& connection, ByteBuffer& inBuffer, OPlayerController*& outPlayer) 
{
	auto it = m_loginChecks.find(connection);
	
	// Already verified basic handshake, just waiting on API response
	if (it != m_loginChecks.end())
	{
		// Request is still out going
		if (it->second == UserCheckState::Pending)
			return NetResponseCode::WaitingOnUpStream;

		if (it->second == UserCheckState::Confirmed)
		{
			m_loginChecks.erase(it);
			return NetResponseCode::Accepted;
		}
		else
		{
			m_loginChecks.erase(it);
			return NetResponseCode::BadAuthentication;
		}
	}

	// New connection entirely
	else 
	{
		string password;
		string userId;
		string sessionId;

		if (!Decode(inBuffer, password) || !Decode(inBuffer, userId) || !Decode(inBuffer, sessionId))
			return NetResponseCode::BadRequest;

		if (!m_password.empty() && m_password != password)
			return NetResponseCode::BadAuthentication;

		// Issue http request to check the user is who they say they are
		m_loginChecks[connection] = UserCheckState::Pending;
		m_controller->VerifyUser(userId, sessionId,
		[this, connection, outPlayer](Http::Response& response)
		{
			if (response.getStatus() == 200)
			{
				json::value raw;
				json::parse(raw, response.getBody());
				if (raw.is<json::object>())
				{
					string displayName = raw.get<json::object>()["displayName"].get<string>();
					outPlayer->SetPlayerName(displayName);
				}
				m_loginChecks[connection] = UserCheckState::Confirmed;
			}
			else
				m_loginChecks[connection] = UserCheckState::Denied;
		});
		return NetResponseCode::WaitingOnUpStream;
	}
}