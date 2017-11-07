#include "Includes\Core\NetRemoteSession.h"
#include "Includes\Core\Engine.h"
#include "Includes\Core\Game.h"



NetRemoteSession::NetRemoteSession(const Engine* engine, const NetIdentity identity) :
	NetSession(engine, identity)
{
	bIsHost = false;
}

NetRemoteSession::~NetRemoteSession()
{
}

bool NetRemoteSession::Start() 
{
	const NetIdentity& remote = GetSessionIdentity();

	if (!m_TcpSocket.Connect(remote))
	{
		LOG_ERROR("Unable to connect to net session (%s:%i). TCP error.", remote.ip.toString().c_str(), remote.port);
		return false;
	}
	if (!m_UdpSocket.Connect(remote))
	{
		LOG_ERROR("Unable to connect to net session (%s:%i). UDP error.", remote.ip.toString().c_str(), remote.port);
		return false;
	}

	m_clientStatus = LocalClientStatus::PreHandshake;
	bIsConnected = true;

	LOG("Remote net session correctly initialized on (%s:%i)", remote.ip.toString().c_str(), remote.port);
	return true;
}

bool NetRemoteSession::SendHandshake() 
{
	ByteBuffer content;

	// Build header
	Encode<Version>(content, m_engine->GetVersionNo());
	Encode<Version>(content, m_engine->GetGame()->GetVersionNo());
	Encode<uint16>(content, NetRequestType::Connect);

	// TODO - Encode password
	// TODO - Encode username

	return m_TcpSocket.Send(content.Data(), content.Size());
}

bool NetRemoteSession::ValidateHandshakeResponse(RawNetPacket& packet) 
{
	uint16 rawCode;
	if (!Decode<uint16>(packet.buffer, rawCode))
		return false;

	m_connectionStatus = (NetResponseCode)rawCode;
	switch (m_connectionStatus)
	{
	// Unknown should never be returned, so just rubbish?
	case Unknown:
		return false;

	case Accepted:
		m_clientStatus = Connected;
		break;

	case Responded:
		break;

	case BadRequest:
		break;

	case Banned:
		break;

	case BadPassword:
		break;

	case BadVersions:
		break;

	case ServerFull:
		break;
	}

	// Received valid response
	return true;
}


void NetRemoteSession::Update(const float& deltaTime) 
{
	// Send handshake
	if (m_clientStatus == PreHandshake)
	{
		if (!SendHandshake())
		{
			LOG_ERROR("Failed to send handshake");
		}
		else
			m_clientStatus = WaitingOnHandshake;
		return;
	}

	// Wait until handshake response has been received
	if (m_clientStatus == WaitingOnHandshake)
	{
		std::vector<RawNetPacket> packets;

		// Attempt to decode handshake response
		if (m_TcpSocket.Poll(packets))
		{
			for (RawNetPacket& p : packets)
			{
				p.buffer.Flip();
				if (ValidateHandshakeResponse(p))
				{
					LOG("Received response from server");
					break;
				}
			}
		}

		// TODO - Process remainder of the packets

		// Wait until handshake has been processed
		if (m_clientStatus == WaitingOnHandshake)
			return;
	}

	// Client has disconnected from the server
	if (m_clientStatus == Disconnected)
	{
		bIsConnected = false;
		return;
	}
}
