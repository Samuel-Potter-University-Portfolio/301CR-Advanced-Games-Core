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
	// Update for any timed out or pending connection 
	for (auto& it = m_connectionLookup.begin(); it != m_connectionLookup.end();)
	{
		// AFK/Idle packet timeout
		if (it->second->state == NetPlayerConnection::State::Connected)
		{
			if ((it->second->inactivityTimer += deltaTime) >= m_maxInactivityTime)
			{
				// Remove controller
				LOG("%s:%i timed out..", it->second->identity.ip.toString().c_str(), it->second->identity.port);
				m_playerControllers.erase(std::remove(m_playerControllers.begin(), m_playerControllers.end(), it->second->controller), m_playerControllers.end());
				OObject::Destroy(it->second->controller);

				delete it->second;
				m_connectionLookup.erase(it++);
				continue;
			}
		}


		// Wait 30 seconds for this connection to do something before just leaving it
		else if (it->second->state == NetPlayerConnection::State::Waiting)
		{

			// Waited too long so ignore this connection
			if ((it->second->inactivityTimer += deltaTime) >= 30.0f)
			{
				// Send handshake response
				ByteBuffer response;
				EncodeHandshakeResponse(NetResponseCode::ServerInternalError, response, nullptr);
				m_TcpSocket.SendTo(response.Data(), response.Size(), it->first);

				// Forcefully destroy as it wasn't added to the game yet
				m_playerControllers.erase(std::remove(m_playerControllers.begin(), m_playerControllers.end(), it->second->controller), m_playerControllers.end());
				it->second->controller->OnDestroy();
				delete it->second->controller;
				delete it->second;
				m_connectionLookup.erase(it++);
				continue;
			}

			// Check with layer to see if now accepting this connection
			else
			{
				ByteBuffer dud;
				NetResponseCode status = m_netLayer->OnDecodeHandshake(it->first, dud, it->second->controller);

				// Player has been accepted (Woo)
				if (status == NetResponseCode::Accepted)
				{
					ByteBuffer response;
					EncodeHandshakeResponse(NetResponseCode::Accepted, response, it->second->controller);
					m_TcpSocket.SendTo(response.Data(), response.Size(), it->first);
					it->second->inactivityTimer = 0; // Reset timer
					LOG("Player(%i) connected from %s:%i", it->second->controller->GetNetworkOwnerID(), it->first.ip.toString().c_str(), it->first.port);
				}

				// Call has immediate response
				else if (status != NetResponseCode::WaitingOnUpStream)
				{
					ByteBuffer response;
					EncodeHandshakeResponse(status, response, nullptr);
					m_TcpSocket.SendTo(response.Data(), response.Size(), it->first);

					// Forcefully destroy as it wasn't added to the game yet
					m_playerControllers.erase(std::remove(m_playerControllers.begin(), m_playerControllers.end(), it->second->controller), m_playerControllers.end());
					it->second->controller->OnDestroy();
					delete it->second->controller;
					delete it->second;
					m_connectionLookup.erase(it++);
					continue;
				}
			}

		}

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
				NetResponseCode status = DecodeHandshake(packet.source, packet.buffer, player);

				// Player has been accepted right now
				if (status == NetResponseCode::Accepted)
				{
					ByteBuffer response;
					EncodeHandshakeResponse(status, response, player);
					m_TcpSocket.SendTo(response.Data(), response.Size(), packet.source);

					playerConnection = new NetPlayerConnection;
					playerConnection->identity = packet.source;
					playerConnection->controller = player;
					playerConnection->state = NetPlayerConnection::State::Connected;
					m_connectionLookup[playerConnection->identity] = playerConnection;
					LOG("Player(%i) connected from %s:%i", player->GetNetworkOwnerID(), packet.source.ip.toString().c_str(), packet.source.port);
				}

				// Connection requires more time to make a decision 
				else if(status == NetResponseCode::WaitingOnUpStream)
				{
					playerConnection = new NetPlayerConnection;
					playerConnection->identity = packet.source;
					playerConnection->controller = player;
					playerConnection->state = NetPlayerConnection::State::Waiting;
					m_connectionLookup[playerConnection->identity] = playerConnection;
				}

				// Call has immediate response
				else
				{
					ByteBuffer response;
					EncodeHandshakeResponse(status, response, nullptr);
					m_TcpSocket.SendTo(response.Data(), response.Size(), packet.source);
				}
			}

			// Player already connected (So just attempt to decode)
			else if(playerConnection->state == NetPlayerConnection::State::Connected)
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
			if (GetPlayerFromIdentity(packet.source, playerConnection) && playerConnection->state == NetPlayerConnection::State::Connected)
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


NetResponseCode NetHostSession::DecodeHandshake(const NetIdentity& source, ByteBuffer& inBuffer, OPlayerController*& outPlayer)
{
	Version engineVersion, gameVersion;
	uint16 rawRequestType;

	// Invalid header
	if (!Decode<Version>(inBuffer, engineVersion) ||
		!Decode<Version>(inBuffer, gameVersion) ||
		!Decode<uint16>(inBuffer, rawRequestType))
		return NetResponseCode::BadRequest;

	// Missmatching versions
	if (GetGame()->GetEngine()->GetVersionNo() != engineVersion || GetGame()->GetVersionNo() != gameVersion)
		return NetResponseCode::BadVersions;
		
	

	// Return appropriate responses for this request
	switch ((NetRequestType)rawRequestType)
	{
		// Acknowledge by pinging back
		case NetRequestType::Ping:
			return NetResponseCode::Responded;


		// Attempt to accept the player
		case NetRequestType::Connect:
		{
			// Server is full
			if (m_playerControllers.size() >= GetMaxPlayerCount())
				return NetResponseCode::ServerFull;


			// Create a new player assuming that the layer will accept (Will clean up later if not)
			outPlayer = GetGame()->playerControllerClass->New<OPlayerController>();
			NetResponseCode response = m_netLayer->OnDecodeHandshake(source, inBuffer, outPlayer);

			if (response == NetResponseCode::WaitingOnUpStream || response == NetResponseCode::Accepted)
				return response;

			// Layer rejected the connection
			else
			{
				delete outPlayer;
				outPlayer = nullptr;
				return response;
			}
		}


		// Return server information
		case NetRequestType::Query:
			return NetResponseCode::Responded;
	}

	return NetResponseCode::Unknown;
}

void NetHostSession::EncodeHandshakeResponse(const NetResponseCode& code, ByteBuffer& outBuffer, OPlayerController* player) 
{
	Encode<uint16>(outBuffer, (uint16)code);

	switch (code)
	{
		case NetResponseCode::Accepted:
		{
			player->m_networkOwnerId = NewPlayerID();
			player->m_networkId = NewObjectID();
			player->bFirstNetUpdate = true;
			player->UpdateRole(this);
			m_playerControllers.emplace_back(player);
			GetGame()->AddObject(player);
			player->OnPostNetInitialize();


			// Encode new player connection information
			Encode<uint16>(outBuffer, player->m_networkOwnerId);
			Encode<uint16>(outBuffer, player->m_networkId);

			// Encode current level info
			LLevel* level = GetGame()->GetCurrentLevel();
			if (level != nullptr)
			{
				Encode<uint16>(outBuffer, level->GetClass()->GetID());
				Encode<uint32>(outBuffer, level->GetInstanceID());
			}
			else
			{
				Encode<uint16>(outBuffer, 0);
				Encode<uint32>(outBuffer, 0);
			}

			player->EncodeSyncVarRequests(player->m_networkOwnerId, outBuffer, TCP, true);
		}


		// Return server information
		case NetResponseCode::Responded:
		{
			Encode<uint16>(outBuffer, m_playerControllers.size());			// Players connected 
			Encode<uint16>(outBuffer, m_maxPlayerCount);					// Player limit
			Encode<string>(outBuffer, "Unnamed Server");					// TODO - Server name
			Encode<uint32>(outBuffer, m_netLayer->GetConnectionBitFlags());	// Connection Bitflags
		}
	}
}