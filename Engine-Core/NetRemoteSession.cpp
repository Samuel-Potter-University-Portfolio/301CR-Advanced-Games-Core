#include "Includes\Core\NetRemoteSession.h"
#include "Includes\Core\Engine.h"
#include "Includes\Core\Game.h"
#include "Includes\Core\Level.h"



NetRemoteSession::NetRemoteSession(Game* game, const NetIdentity identity) :
	NetSession(game, identity)
{
	bIsHost = false;
}

NetRemoteSession::~NetRemoteSession()
{
	LOG("NetRemoteSession destroyed.");
}

bool NetRemoteSession::Start() 
{
	const NetIdentity& remote = GetSessionIdentity();

	if (!m_TcpSocket.Connect(remote))
	{
		LOG_ERROR("Unable to connect to net session (%s:%i). TCP error.", remote.ip.toString().c_str(), remote.port);
		return false;
	}
	if (!m_UdpSocket.ConnectAs(remote, m_TcpSocket.GetLocalIdentity()))
	{
		LOG_ERROR("Unable to connect to net session (%s:%i). UDP error.", remote.ip.toString().c_str(), remote.port);
		return false;
	}

	m_clientStatus = LocalClientStatus::PreHandshake;
	bIsConnected = true;

	LOG("Remote net session correctly initialized on (%s:%i)", remote.ip.toString().c_str(), remote.port);
	return true;
}

void NetRemoteSession::NetUpdate(const float& deltaTime)
{
	if (!EnsureConnection())
		return;


	// Check to see if timed out
	if ((m_inactivityTimer += deltaTime) >= m_maxInactivityTime)
	{
		LOG("Server connection timed out..");
		OObject::Destroy(m_localController);
		bIsConnected = false;
		GetGame()->SwitchLevel(GetGame()->defaultLevel);
	}


	////
	// Read and attempt to connect/decode any data retrieved from server
	////
	std::vector<RawNetPacket> packets;

	// Fetch TCP packets
	if (m_TcpSocket.Poll(packets))
		for (RawNetPacket& packet : packets)
		{
			packet.buffer.Flip();
			DecodeNetUpdate(nullptr, packet.buffer, TCP);
			m_inactivityTimer = 0;
		}

	// Fetch UDP packets
	packets.clear();
	if (m_UdpSocket.Poll(packets))
		for (RawNetPacket& packet : packets)
		{
			packet.buffer.Flip();
			DecodeNetUpdate(nullptr, packet.buffer, UDP);
			m_inactivityTimer = 0;
		}



	////
	// Encode any data to send to the server
	////
	ByteBuffer tcpContent;
	ByteBuffer udpContent;

	EncodeNetUpdate(nullptr, tcpContent, TCP);
	EncodeNetUpdate(nullptr, udpContent, UDP);

	const NetIdentity& identity = GetSessionIdentity();
	m_TcpSocket.SendTo(tcpContent.Data(), tcpContent.Size(), identity); // Will return false in event of disconnect, so could use this?
	m_UdpSocket.SendTo(udpContent.Data(), udpContent.Size(), identity);
}

bool NetRemoteSession::EnsureConnection() 
{
	// Send handshake if not connected
	if (m_clientStatus == PreHandshake)
	{
		ByteBuffer content;
		EncodeHandshake(content);
		if (!m_TcpSocket.Send(content.Data(), content.Size()))
		{
			LOG_ERROR("Failed to send handshake");
		}
		else
			m_clientStatus = WaitingOnHandshake;
		return false;
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
				switch (DecodeHandshakeResponse(p.buffer, m_localController))
				{
					m_clientStatus = Disconnected;

					// Unknown should never be returned, so just rubbish?
					case NetResponseCode::Unknown:
						LOG_ERROR("Unknown connection error");
						break;

					case NetResponseCode::Accepted:
						LOG("Connected to server as Player(%i)", m_localController->GetNetworkOwnerID());
						m_clientStatus = Connected;
						break;

					case NetResponseCode::Responded:
						LOG("Server acknowledged");
						break;

					case NetResponseCode::BadRequest:
						LOG_ERROR("BadRequest connecting to server");
						break;

					case NetResponseCode::Banned:
						LOG("Banned from server");
						break;

					case NetResponseCode::BadAuthentication:
						LOG("Bad authentication provided when connecting to server");
						break;

					case NetResponseCode::BadVersions:
						LOG("Version missmatch between client and server");
						break;

					case NetResponseCode::ServerInternalError:
						LOG("Server internal error");
						break;

					case NetResponseCode::ServerFull:
						LOG("Server full"); 
						break;
				}
			}
		}

		// Wait until handshake has been processed
		if (m_clientStatus == WaitingOnHandshake)
			return false;
	}


	// Client has disconnected from the server
	if (m_clientStatus == Disconnected)
	{
		bIsConnected = false;
		return false;
	}

	return true;
}


void NetRemoteSession::EncodeHandshake(ByteBuffer& outBuffer)
{
	// Version numbers
	Encode(outBuffer, GetGame()->GetEngine()->GetVersionNo());
	Encode(outBuffer, GetGame()->GetVersionNo());

	// Request type
	Encode<uint16>(outBuffer, (uint16)NetRequestType::Connect);

	// Let layer encode any extra data
	m_netLayer->OnEncodeHandshake(GetSessionIdentity(), outBuffer);
}


NetResponseCode NetRemoteSession::DecodeHandshakeResponse(ByteBuffer& inBuffer, OPlayerController*& outPlayer)
{
	uint16 rawResponse;
	if (!Decode<uint16>(inBuffer, rawResponse))
		return NetResponseCode::Unknown;

	const NetResponseCode response = (NetResponseCode)rawResponse;

	// Retrieve connection info
	if (response == NetResponseCode::Accepted)
	{
		uint16 netId;
		uint16 controllerId;

		// Decode information
		if (!Decode<uint16>(inBuffer, netId) ||
			!Decode<uint16>(inBuffer, controllerId) 
		)
		{
			LOG_ERROR("Server's response to handshake is unparsable.");
			return NetResponseCode::ServerInternalError;
		}


		// Use an existing controller
		auto playerList = GetGame()->GetActiveObjects<OPlayerController>();
		if (playerList.size() >= 1)
		{
			outPlayer = playerList[0];

			// Cleaup any other controllers that may exist for some reason
			for (uint32 i = 1; i < playerList.size(); ++i)
				OObject::Destroy(playerList[i]);

			// As we are reusing this controller, let level cleanup
			LLevel* level = GetGame()->GetCurrentLevel();
			if (level != nullptr)
				level->GetLevelController()->OnPlayerDisconnect(outPlayer);
		}
		else if (playerList.size() == 0)
			outPlayer = GetGame()->playerControllerClass->New<OPlayerController>();


		// Setup new player controller
		m_sessionNetId = netId;
		outPlayer->m_networkOwnerId = netId;
		outPlayer->m_networkId = controllerId;
		outPlayer->bFirstNetUpdate = true;
		outPlayer->UpdateRole(this);
		outPlayer->DecodeSyncVarRequests(0, inBuffer, TCP, true);
		GetGame()->AddObject(outPlayer);
		outPlayer->OnPostNetInitialize();
	}

	return response;
}