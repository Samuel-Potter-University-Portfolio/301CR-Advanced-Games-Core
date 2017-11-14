#include "Includes\Core\NetSession.h"
#include "Includes\Core\NetController.h"

#include "Includes\Core\Engine.h"
#include "Includes\Core\Game.h"
#include "Includes\Core\Level.h"
#include "Includes\Core\Logger.h"



NetSession::NetSession(const Engine* engine, const NetIdentity identity) :
	m_engine(engine), m_netIdentity(identity), m_netId(m_playerIdCounter++)
{
	m_entityIdCounter = 1; // 0 - Reservered as nullptr
}

NetSession::~NetSession()
{
}

void NetSession::HandleUpdate(const float& deltaTime)
{
	// Only tick at desired rate
	m_tickTimer += deltaTime;
	if (m_tickTimer < m_sleepRate )
		return;

	m_newPlayers.clear();
	Update(m_tickTimer);
	m_tickTimer -= m_sleepRate;
}


void NetSession::EncodeClientHandshake(ByteBuffer& buffer) 
{
	// Build header
	Encode<Version>(buffer, m_engine->GetVersionNo());
	Encode<Version>(buffer, m_engine->GetGame()->GetVersionNo());
	Encode<uint16>(buffer, (uint16)NetRequestType::Connect);

	// TODO - Encode password
	// TODO - Encode username
}

NetResponseCode NetSession::DecodeClientHandshake(ByteBuffer& inbuffer, ByteBuffer& outBuffer, uint16& outNetId)
{
	// Check message header
	uint16 requestType;
	{
		Version engineVersion;
		Version gameVersion;

		// Invalid header
		if (!Decode<Version>(inbuffer, engineVersion) ||
			!Decode<Version>(inbuffer, gameVersion) ||
			!Decode<uint16>(inbuffer, requestType))
		{
			Encode<uint16>(outBuffer, (uint16)NetResponseCode::BadRequest);
			return NetResponseCode::BadRequest;
		}

		// Invalid versions
		if (m_engine->GetVersionNo() != engineVersion || m_engine->GetGame()->GetVersionNo() != gameVersion)
		{
			Encode<uint16>(outBuffer, (uint16)NetResponseCode::BadVersions);
			return NetResponseCode::BadVersions;
		}

		// TODO - Ban checks
		// TODO - Password checks
		// TODO - Whitelist checks
		// TODO - Server is full checks
	}


	// Perform any additional checks/payload filling
	NetRequestType request = (NetRequestType)requestType;

	switch (request)
	{
		// User is just pinging the server to get a response
		case NetRequestType::Ping:
			Encode<uint16>(outBuffer, (uint16)NetResponseCode::Responded);
			return NetResponseCode::Responded;


		// The user is attempting to connect to the server as a player
		case NetRequestType::Connect:
			outNetId = m_playerIdCounter++;
			Encode<uint16>(outBuffer, (uint16)NetResponseCode::Accepted);
			Encode<uint16>(outBuffer, outNetId);
			m_newPlayers.emplace(outNetId); // Add to new players to make sure correct welcome packets get sent

			// TODO - Fill in more session information
			// TODO - Notify player of level and entity info

			return NetResponseCode::Accepted;


		// The user is querying for server details
		case NetRequestType::Query:
		
			Encode<uint16>(outBuffer, (uint16)NetResponseCode::Responded);

			// TODO - query stuff
			//Encode<uint16>(outBuffer, m_players.size());		// Players connected 
			//Encode<uint16>(outBuffer, maxPlayerCount);		// Player limit
			//Encode<string>(outBuffer, "Unnamed Server");		// TODO - Server name
			//Encode<uint8>(outBuffer, 0);						// TODO - Bitflags
			return NetResponseCode::Responded;
	}

	return NetResponseCode::Unknown;
}

NetResponseCode NetSession::DecodeServerHandshake(ByteBuffer& buffer, uint16& outNetId)
{
	uint16 rawCode;
	if (!Decode<uint16>(buffer, rawCode))
		return NetResponseCode::Unknown;

	const NetResponseCode response = (NetResponseCode)rawCode;

	// Get net id if successful
	if (response == NetResponseCode::Accepted)
	{
		Decode<uint16>(buffer, outNetId);
	}
	return response;
}


void NetSession::EncodeEntityMessage(const uint16& targetNetId, ByteBuffer& buffer, const SocketType& socketType, Entity* entity)
{
	// TODO - UPDATE
	/*
	if ((!IsHost() && !entity->HasNetControl()) || !entity->IsNetSynced()) 
		return;

	bool newConnection = (m_newPlayers.find(targetNetId) != m_newPlayers.end());

	// If entity has no id, it must be newly spawned
	if(entity->GetNetworkID() == 0 || newConnection)
	{
		// Major Entity changes must happen over TCP 
		if (socketType == UDP)
			return;


		// Was part of level
		if (entity->WasLoadedWithLevel())
		{
			if(!newConnection || entity->GetNetworkID() == 0)
				entity->m_networkId = m_entityIdCounter++; // Give a unique net id

			// Update net role
			if (IsHost())
				entity->m_netRole = (GetNetworkID() == entity->GetNetworkOwnerID() ? NetRole::HostOwner : NetRole::HostPuppet);
			else
				entity->m_netRole = (GetNetworkID() == entity->GetNetworkOwnerID() ? NetRole::RemoteOwner : NetRole::RemotePuppet);

			Encode<uint8>(buffer, (uint8)NetEntityMethod::ControlOverride);
			Encode<uint16>(buffer, entity->m_networkId);
			Encode<uint16>(buffer, entity->m_networkOwnerId);
			Encode<uint32>(buffer, entity->GetInstanceID());
		}
		// Was dynamically spawned
		else
		{
			if (!newConnection || entity->GetNetworkID() == 0)
				entity->m_networkId = m_entityIdCounter++; // Give a unique net id

			// Update net role
			if (IsHost())
				entity->m_netRole = (GetNetworkID() == entity->GetNetworkOwnerID() ? NetRole::HostOwner : NetRole::HostPuppet);
			else
				entity->m_netRole = (GetNetworkID() == entity->GetNetworkOwnerID() ? NetRole::RemoteOwner : NetRole::RemotePuppet);

			Encode<uint8>(buffer, (uint8)NetEntityMethod::Spawn);
			Encode<uint16>(buffer, entity->m_networkId);
			Encode<uint16>(buffer, entity->m_networkOwnerId);
			Encode<uint32>(buffer, entity->GetTypeID());
		}
	}

	// Encode any changes to the entity
	else
	{
		Encode<uint8>(buffer, (uint8)NetEntityMethod::Update);
		Encode<uint16>(buffer, entity->m_networkId);
		// TODO - Sync vars
		entity->EncodeRPCRequests(targetNetId, buffer, socketType);
	}
	*/
}

void NetSession::DecodeEntityMessage(const uint16& sourceNetId, ByteBuffer& buffer, const SocketType& socketType)
{
	// TODO - UPDATE
	/*
	uint8 rawMethod;
	Decode(buffer, rawMethod);
	NetEntityMethod method = (NetEntityMethod)rawMethod;

	switch (method)
	{
	case NetEntityMethod::Nothing:
		break;


	// Attempt to spawn new entity
	case NetEntityMethod::Spawn:
	{
		uint16 netId;
		uint16 netOwnerId;
		uint32 typeId;

		if (!Decode(buffer, netId) ||
			!Decode(buffer, netOwnerId) ||
			!Decode(buffer, typeId)
			) return;

		// Fetch factory and perform checkss
		ClassFactory<Entity>* factory = m_engine->GetGame()->GetEntityFactoryFromID(typeId);
		if (IsHost())
		{
			LOG_WARNING("Received entity spawn request from client");
			return;
		}
		if (factory == nullptr)
		{
			LOG_ERROR("Cannot spawn entity of id:%i (Cannot find factory)", typeId);
			return;
		}

		// Spawn in entity with given settings
		Entity* e = factory->New();
		e->m_networkId = netId;
		e->m_networkOwnerId = netOwnerId;
		e->m_netRole = (netOwnerId == GetNetworkID() ? NetRole::RemoteOwner : NetRole::RemotePuppet); // Deduce role
		m_engine->GetGame()->GetCurrentLevel()->AddEntity(e);
		break;
	}


	case NetEntityMethod::Despawn:
		break;

	// Attempt to override controll of entity, and give it correct net id
	case NetEntityMethod::ControlOverride:
	{
		uint16 netId;
		uint16 netOwnerId;
		uint32 instanceId;

		if (!Decode(buffer, netId) ||
			!Decode(buffer, netOwnerId) ||
			!Decode(buffer, instanceId)
			) return;

		// Perform checkss
		if (IsHost())
		{
			LOG_WARNING("Received entity control override request from client");
			return;
		}

		Entity* e = m_engine->GetGame()->GetCurrentLevel()->GetEntityFromInstanceId(instanceId);
		if (e == nullptr)
		{
			LOG_ERROR("Cannot let host override control, as no entity with instance id %i can be found", instanceId);
			return;
		}

		e->m_networkId = netId;
		e->m_networkOwnerId = netOwnerId;
		e->m_netRole = (netOwnerId == GetNetworkID() ? NetRole::RemoteOwner : NetRole::RemotePuppet); // Deduce role
		m_engine->GetGame()->GetCurrentLevel()->m_netEntities[netId] = e; // Add to lookup table
		break;
	}

	case NetEntityMethod::Update:
	{
		uint16 netId;
		if (!Decode(buffer, netId)) return;

		Entity* entity = m_engine->GetGame()->GetCurrentLevel()->GetEntityFromNetId(netId);
		if (entity == nullptr)
			return;

		// TODO - Sync vars;
		entity->DecodeRPCRequests(sourceNetId, buffer, socketType);
		break;
	}

	default:
		break;
	}
	*/
}

void NetSession::NetDecode(const uint16& netId, ByteBuffer& buffer, const SocketType& socketType)
{
	while (buffer.Size() != 0)
	{
		uint8 rawMessage;
		Decode(buffer, rawMessage);
		NetMessage messageType = (NetMessage)rawMessage;

		switch (messageType)
		{
			case NetMessage::Nothing:
				break;

			case NetMessage::EntityMessage:
				DecodeEntityMessage(netId, buffer, socketType);
				break;

			default:
				break;
		}
	}
}