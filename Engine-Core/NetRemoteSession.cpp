#include "Includes\Core\NetRemoteSession.h"
#include "Includes\Core\Engine.h"
#include "Includes\Core\Game.h"
#include "Includes\Core\Level.h"
#include "Includes\Core\Entity.h"



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

void NetRemoteSession::Update(const float& deltaTime)
{
	if (!EnsureConnection())
		return;

	////
	// Read and attempt to connect/decode any data retrieved from server
	////
	std::vector<RawNetPacket> packets;

	// Fetch TCP packets
	if (m_TcpSocket.Poll(packets))
		for (RawNetPacket& packet : packets)
		{
			packet.buffer.Flip();
			NetDecode(0, packet.buffer, TCP);
		}

	// Fetch UDP packets
	packets.clear();
	if (m_UdpSocket.Poll(packets))
		for (RawNetPacket& packet : packets)
		{
			packet.buffer.Flip();
			NetDecode(0, packet.buffer, UDP);
		}



	////
	// Encode any data to send to the server
	////
	ByteBuffer tcpContent;
	ByteBuffer udpContent;

	NetEncode(tcpContent, TCP);
	NetEncode(udpContent, UDP);

	const NetIdentity& identity = GetSessionIdentity();
	if (!m_TcpSocket.SendTo(tcpContent.Data(), tcpContent.Size(), identity))
	{
		// TODO - Handle disconnections 
		LOG_ERROR("Failed to send TCP update to %s:%i", identity.ip.toString(), identity.port);
	}
	if (!m_UdpSocket.SendTo(udpContent.Data(), udpContent.Size(), identity))
		LOG_ERROR("Failed to send UDP update to %s:%i", identity.ip.toString(), identity.port); // Will never happen, as connectionless
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
	case NetResponseCode::Unknown:
		LOG_ERROR("Unknown connection error");
		return false;

	case NetResponseCode::Accepted:
		m_clientStatus = Connected;
		LOG("Connected to server");
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

	case NetResponseCode::BadPassword:
		LOG("Bad password provided when connecting to server");
		break;

	case NetResponseCode::BadVersions:
		LOG("Version missmatch between client and server");
		break;

	case NetResponseCode::ServerFull:
		LOG("Server full");
		break;
	}

	// Received valid response
	return true;
}

bool NetRemoteSession::EnsureConnection() 
{
	// Send handshake if not connected
	if (m_clientStatus == PreHandshake)
	{
		ByteBuffer content;
		EncodeClientHandshake(content);
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
				if (ValidateHandshakeResponse(p))
					break;
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

void NetRemoteSession::NetEncode(ByteBuffer& buffer, const SocketType& socketType) 
{
	// Encode all owned entities
	for (Entity* entity : m_engine->GetGame()->GetCurrentLevel()->GetEntities())
		if (entity->IsNetSynced() && entity->IsNetOwner())
			EncodeEntityMessage(buffer, socketType, entity);


	// Encode empty ping packet
	if (buffer.Size() == 0)
		Encode<uint8>(buffer, (uint8)NetMessage::Nothing);
}