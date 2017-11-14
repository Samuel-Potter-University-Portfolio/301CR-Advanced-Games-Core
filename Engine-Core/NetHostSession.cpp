#include "Includes\Core\NetHostSession.h"
#include "Includes\Core\Engine.h"
#include "Includes\Core\Game.h"
#include "Includes\Core\Level.h"


NetHostSession::NetHostSession(const Engine* engine, const NetIdentity identity) :
	NetSession(engine, identity)
{
	bIsHost = true;
}

NetHostSession::~NetHostSession()
{
	for (NetPlayer* c : m_players)
		delete c;

	LOG("NetHostSession closed.");
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

void NetHostSession::Update(const float& deltaTime)
{
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
			NetPlayer* player;
			if (!GetPlayerFromIdentity(packet.source, player)) 
			{
				// Attempt to accept with handshake
				player = new NetPlayer;
				player->m_identity = packet.source;
				ByteBuffer response;
				if (DecodeClientHandshake(packet.buffer, response, player->m_uniqueId) == NetResponseCode::Accepted)
				{
					// Add player
					m_players.emplace_back(player);
					m_playerLookup[player->m_identity] = player;
					LOG("Player(%i) connected from %s:%i", player->GetUniqueID(), player->m_identity.ip.toString().c_str(), player->m_identity.port);
				}

				// Failed handshake
				else
					delete player;

				// Send handshake response
				m_TcpSocket.SendTo(response.Data(), response.Size(), packet.source);
			}

			// Player already connected (So just attempt to decode)
			else
				NetDecode(player->GetUniqueID(), packet.buffer, TCP);
		}

	// Fetch UDP packets
	packets.clear();
	if (m_UdpSocket.Poll(packets))
		for (RawNetPacket& packet : packets)
		{
			packet.buffer.Flip();

			// Player connected (Only perform handshake on TCP)
			NetPlayer* player;
			if (GetPlayerFromIdentity(packet.source, player))
				NetDecode(player->GetUniqueID(), packet.buffer, UDP);
		}

	

	////
	// Encode any data and broadcast to all connected clients
	////
	ByteBuffer tcpContent;
	ByteBuffer udpContent;

	// Send out packet update
	for (NetPlayer* player : m_players)
	{
		tcpContent.Clear();
		udpContent.Clear();
		NetEncode(player->GetUniqueID(), tcpContent, TCP);
		NetEncode(player->GetUniqueID(), udpContent, UDP);

		const NetIdentity& identity = player->m_identity;
		if (!m_TcpSocket.SendTo(tcpContent.Data(), tcpContent.Size(), identity))
		{
			// TODO - Handle disconnections 
			LOG_ERROR("Failed to send TCP update to %s:%i", identity.ip.toString().c_str(), identity.port);
		}
		if (!m_UdpSocket.SendTo(udpContent.Data(), udpContent.Size(), identity))
			LOG_ERROR("Failed to send UDP update to %s:%i", identity.ip.toString().c_str(), identity.port); // Will never happen, as connectionless
	}


	// Clear any queued net data
	// TODO - UPDATE
	//for (Entity* entity : m_engine->GetGame()->GetCurrentLevel()->GetEntities())
	//	if (entity->IsNetSynced())
	//		entity->ClearQueuedNetData();
}

bool NetHostSession::GetPlayerFromIdentity(const NetIdentity& identity, NetPlayer*& outPlayer) const
{
	auto it = m_playerLookup.find(identity);
	if (it == m_playerLookup.end())
		return false;
	
	outPlayer = it->second;
	return true;
}

void NetHostSession::NetEncode(const uint16& netId, ByteBuffer& buffer, const SocketType& socketType)
{
	// Encode all entities
	// TODO - UPDATE
	/*
	ByteBuffer tempBuffer;
	for (Entity* entity : m_engine->GetGame()->GetCurrentLevel()->GetEntities())
		if (entity->IsNetSynced())
		{
			EncodeEntityMessage(netId, tempBuffer, socketType, entity);
			if (tempBuffer.Size() != 0)
			{
				Encode<uint8>(buffer, (uint8)NetMessage::EntityMessage);
				buffer.Push(tempBuffer.Data(), tempBuffer.Size());
				tempBuffer.Clear();
			}
		}
		*/

	// Encode empty ping packet
	if (buffer.Size() == 0)
		Encode<uint8>(buffer, (uint8)NetMessage::Nothing);
}