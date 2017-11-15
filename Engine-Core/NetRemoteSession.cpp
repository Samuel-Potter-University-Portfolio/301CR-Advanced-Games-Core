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

	////
	// Read and attempt to connect/decode any data retrieved from server
	////
	std::vector<RawNetPacket> packets;

	// Fetch TCP packets
	if (m_TcpSocket.Poll(packets))
		for (RawNetPacket& packet : packets)
		{
			packet.buffer.Flip();
			NetDecode(nullptr, packet.buffer, TCP);
		}

	// Fetch UDP packets
	packets.clear();
	if (m_UdpSocket.Poll(packets))
		for (RawNetPacket& packet : packets)
		{
			packet.buffer.Flip();
			NetDecode(nullptr, packet.buffer, UDP);
		}



	////
	// Encode any data to send to the server
	////
	ByteBuffer tcpContent;
	ByteBuffer udpContent;

	NetEncode(nullptr, tcpContent, TCP);
	NetEncode(nullptr, udpContent, UDP);

	const NetIdentity& identity = GetSessionIdentity();
	if (!m_TcpSocket.SendTo(tcpContent.Data(), tcpContent.Size(), identity))
	{
		// TODO - Handle disconnections 
		LOG_ERROR("Failed to send TCP update to %s:%i", identity.ip.toString().c_str(), identity.port);
	}
	if (!m_UdpSocket.SendTo(udpContent.Data(), udpContent.Size(), identity))
		LOG_ERROR("Failed to send UDP update to %s:%i", identity.ip.toString().c_str(), identity.port); // Will never happen, as connectionless
}

bool NetRemoteSession::EnsureConnection() 
{
	// Send handshake if not connected
	if (m_clientStatus == PreHandshake)
	{
		ByteBuffer content;
		EncodeHandshake_ClientToServer(content);
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
				switch (DecodeHandshake_ServerToClient(p.buffer, m_localController))
				{
					// Unknown should never be returned, so just rubbish?
				case NetResponseCode::Unknown:
					LOG_ERROR("Unknown connection error");
					break;

				case NetResponseCode::Accepted:
					m_clientStatus = Connected;
					LOG("Connected to server as Player(%i)", m_localController->GetNetworkOwnerID());
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