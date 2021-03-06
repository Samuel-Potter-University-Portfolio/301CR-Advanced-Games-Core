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
						m_clientStatus = Disconnected;
						break;

					case NetResponseCode::BadRequest:
						LOG_ERROR("BadRequest connecting to server");
						m_clientStatus = Disconnected;
						break;

					case NetResponseCode::Banned:
						LOG("Banned from server");
						m_clientStatus = Disconnected;
						break;

					case NetResponseCode::BadAuthentication:
						LOG("Bad authentication provided when connecting to server");
						m_clientStatus = Disconnected;
						break;

					case NetResponseCode::BadVersions:
						LOG("Version missmatch between client and server");
						m_clientStatus = Disconnected;
						break;

					case NetResponseCode::ServerInternalError:
						LOG("Server internal error");
						m_clientStatus = Disconnected;
						break;

					case NetResponseCode::ServerFull:
						LOG("Server full");
						m_clientStatus = Disconnected;
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
		uint16 netOwnerId;
		uint16 netControllerId;
		uint16 playerLimit;
		string serverName;

		// Decode information
		if (!Decode<uint16>(inBuffer, netOwnerId) ||
			!Decode<uint16>(inBuffer, netControllerId) ||
			!Decode<uint16>(inBuffer, playerLimit) ||
			!Decode<string>(inBuffer, serverName)
		)
		{
			LOG_ERROR("Server's response to handshake is unparsable.");
			return NetResponseCode::ServerInternalError;
		}

		m_sessionName = serverName;
		m_maxPlayerCount = playerLimit;


		// Remove existing controllers
		std::vector<OPlayerController*> players = GetGame()->GetActiveObjects<OPlayerController>();

		for (uint32 i = 0; i < players.size(); ++i)
			OObject::Destroy(players[i]);
		


		// Setup new player controller
		outPlayer = GetGame()->playerControllerClass->New<OPlayerController>();
		m_sessionNetId = netOwnerId;
		outPlayer->m_networkOwnerId = netOwnerId;
		outPlayer->m_networkId = netControllerId;
		outPlayer->m_decodingContext = GetGame();
		outPlayer->bFirstNetUpdate = true;
		outPlayer->UpdateRole(this);
		outPlayer->DecodeSyncVarRequests(0, inBuffer, TCP, true);
		GetGame()->AddObject(outPlayer);

		// Reset level instance id so sever may sync
		LLevel::s_instanceCounter = 0;
		LLevel* level = GetGame()->GetCurrentLevel();
		level->m_instanceId = 0;


		outPlayer->OnPostNetInitialize();
	}

	return response;
}