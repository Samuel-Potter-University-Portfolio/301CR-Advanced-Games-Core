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
				if (VerifyHandshake(player, packet, m_TcpSocket) == NetResponseCode::Accepted)
				{
					// Add player
					m_players.emplace_back(player);
					m_playerLookup[player->m_identity] = player;
					LOG("Player(%i) connected from %s:%i", player->GetUniqueID(), player->m_identity.ip.toString().c_str(), player->m_identity.port);
				}

				// Failed handshake
				else
					delete player;
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

	NetEncode(tcpContent, TCP);
	NetEncode(udpContent, UDP);

	// Send out packet update
	for (NetPlayer* player : m_players)
	{
		const NetIdentity& identity = player->m_identity;
		if (!m_TcpSocket.SendTo(tcpContent.Data(), tcpContent.Size(), identity))
		{
			// TODO - Handle disconnections 
			LOG_ERROR("Failed to send TCP update to %s:%i", identity.ip.toString(), identity.port);
		}
		if (!m_UdpSocket.SendTo(udpContent.Data(), udpContent.Size(), identity))
			LOG_ERROR("Failed to send UDP update to %s:%i", identity.ip.toString(), identity.port); // Will never happen, as connectionless
	}
}

NetResponseCode NetHostSession::VerifyHandshake(NetPlayer* player, RawNetPacket& packet, NetSocket& socket)
{
	// Check message header
	uint16 requestType;
	{
		Version engineVersion;
		Version gameVersion;

		// Invalid header
		if (!Decode<Version>(packet.buffer, engineVersion) ||
			!Decode<Version>(packet.buffer, gameVersion) ||
			!Decode<uint16>(packet.buffer, requestType))
		{
			ByteBuffer response;
			Encode<uint16>(response, (uint16)NetResponseCode::BadRequest);
			socket.SendTo(response.Data(), response.Size(), packet.source);
			return NetResponseCode::BadRequest;
		}

		// Invalid versions
		if (m_engine->GetVersionNo() != engineVersion || m_engine->GetGame()->GetVersionNo() != gameVersion)
		{
			ByteBuffer response;
			Encode<uint16>(response, (uint16)NetResponseCode::BadVersions);
			socket.SendTo(response.Data(), response.Size(), packet.source);
			return NetResponseCode::BadVersions;
		}

		// TODO - Ban checks
		// TODO - Password checks
		// TODO - Whitelist checks
		// TODO - Server is full checks
	}


	// Perform any additional checks/payload filling
	ByteBuffer response;
	NetRequestType request = (NetRequestType)requestType;

	switch (request)
	{
		// User is just pinging the server to get a response
		case NetRequestType::Ping:
			Encode<uint16>(response, (uint16)NetResponseCode::Responded);

			socket.SendTo(response.Data(), response.Size(), packet.source);
			return NetResponseCode::Responded;


		// The user is attempting to connect to the server as a player
		case NetRequestType::Connect:
			Encode<uint16>(response, (uint16)NetResponseCode::Accepted);

			// Set player ID information
			player->m_identity = packet.source;
			player->m_uniqueId = m_playerIdCounter++;

			socket.SendTo(response.Data(), response.Size(), packet.source);
			return NetResponseCode::Accepted;


		// The user is querying for server details
		case NetRequestType::Query:
			Encode<uint16>(response, (uint16)NetResponseCode::Responded);
			Encode<uint16>(response, m_players.size());		// Players connected 
			Encode<uint16>(response, maxPlayerCount);		// Player limit
			Encode<string>(response, "Unnamed Server");		// TODO - Server name
			Encode<uint8>(response, 0);						// TODO - Bitflags

			socket.SendTo(response.Data(), response.Size(), packet.source);
			return NetResponseCode::Responded;
	}

	return NetResponseCode::Unknown;
}

bool NetHostSession::GetPlayerFromIdentity(const NetIdentity& identity, NetPlayer*& outPlayer) const
{
	auto it = m_playerLookup.find(identity);
	if (it == m_playerLookup.end())
		return false;
	
	outPlayer = it->second;
	return true;
}

void NetHostSession::NetEncode(ByteBuffer& buffer, const SocketType& socketType)
{

	// Encode empty ping packet
	if (buffer.Size() == 0)
		Encode<uint8>(buffer, (uint8)NetMessage::Nothing);
}