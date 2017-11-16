#include "Includes\Core\NetHostSession.h"
#include "Includes\Core\Engine.h"
#include "Includes\Core\Game.h"
#include "Includes\Core\Level.h"


NetHostSession::NetHostSession(Game* game, const NetIdentity identity) :
	NetSession(game, identity)
{
	bIsHost = true;
}

NetHostSession::~NetHostSession()
{
	LOG("NetHostSession destroyed.");
}

bool NetHostSession::Start() 
{
	// Setup listeners
	const NetIdentity& host = GetSessionIdentity();

	if (!m_TcpSocket.Listen(host))
	{
		LOG_ERROR("Unable to open net session on (%s:%i). TCP socket not openned (Maybe the port is already in use).", host.ip.toString().c_str(), host.port);
		return false;
	}
	if (!m_UdpSocket.Listen(host))
	{
		LOG_ERROR("Unable to open net session on (%s:%i). UDP socket not openned (Maybe the port is already in use).", host.ip.toString().c_str(), host.port);
		return false;
	}

	bIsConnected = true;
	LOG("Host net session openned on (%s:%i)", host.ip.toString().c_str(), host.port);
	return true;
}

void NetHostSession::NetUpdate(const float& deltaTime)
{
	// Update for any timed out connection
	for (auto& it = m_connectionLookup.begin(); it != m_connectionLookup.end();)
	{
		if ((it->second->inactivityTimer += deltaTime) >= m_maxInactivityTime)
		{
			LOG("%s:%i timed out..", it->second->identity.ip.toString().c_str(), it->second->identity.port);
			OObject::Destroy(it->second->controller);
			delete it->second;
			m_connectionLookup.erase(it++);
		}
		else
			++it;
	}


	////
	// Read and attempt to connect/decode any data retrieved from clients
	////
	std::vector<RawNetPacket> packets;

	// Fetch TCP packets
	if (m_TcpSocket.Poll(packets))
		for (RawNetPacket& packet : packets)
		{
			packet.buffer.Flip();

			// Player not connected
			NetPlayerConnection* playerConnection;
			if (!GetPlayerFromIdentity(packet.source, playerConnection))
			{
				// Attempt to accept with handshake
				OPlayerController* player;
				ByteBuffer response;
				if (DecodeHandshake_ClientToServer(packet.buffer, response, player) == NetResponseCode::Accepted)
				{
					playerConnection = new NetPlayerConnection;
					playerConnection->identity = packet.source;
					playerConnection->controller = player;
					m_connectionLookup[playerConnection->identity] = playerConnection;
					LOG("Player(%i) connected from %s:%i", player->GetNetworkOwnerID(), packet.source.ip.toString().c_str(), packet.source.port);
				}

				// Send handshake response
				m_TcpSocket.SendTo(response.Data(), response.Size(), packet.source);
			}

			// Player already connected (So just attempt to decode)
			else
			{
				NetDecode(playerConnection->controller, packet.buffer, TCP);
				playerConnection->inactivityTimer = -deltaTime;
			}
		}

	// Fetch UDP packets
	packets.clear();
	if (m_UdpSocket.Poll(packets))
		for (RawNetPacket& packet : packets)
		{
			packet.buffer.Flip();

			// Player connected (Only perform handshake on TCP)
			NetPlayerConnection* playerConnection;
			if (GetPlayerFromIdentity(packet.source, playerConnection))
			{
				NetDecode(playerConnection->controller, packet.buffer, UDP);
				playerConnection->inactivityTimer = -deltaTime;
			}
		}

	

	////
	// Send out updates to connected clients
	////
	ByteBuffer tcpContent;
	ByteBuffer udpContent;

	// Send out packet update
	for (auto& it : m_connectionLookup)
	{
		tcpContent.Clear();
		udpContent.Clear();
		NetEncode(it.second->controller, tcpContent, TCP);
		NetEncode(it.second->controller, udpContent, UDP);

		const NetIdentity& identity = it.first;
		m_TcpSocket.SendTo(tcpContent.Data(), tcpContent.Size(), identity); // Will return false in event of disconnect, so could use this?
		m_UdpSocket.SendTo(udpContent.Data(), udpContent.Size(), identity);
	}
}

bool NetHostSession::GetPlayerFromIdentity(const NetIdentity& identity, NetPlayerConnection*& outPlayer) const
{
	auto it = m_connectionLookup.find(identity);
	if (it == m_connectionLookup.end())
		return false;
	
	outPlayer = it->second;
	return true;
}