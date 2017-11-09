#include "Includes\Core\NetSession.h"
#include "Includes\Core\NetController.h"

#include "Includes\Core\Entity.h"
#include "Includes\Core\Engine.h"
#include "Includes\Core\Game.h"
#include "Includes\Core\Level.h"
#include "Includes\Core\Logger.h"



NetSession::NetSession(const Engine* engine, const NetIdentity identity) :
	m_engine(engine), m_netIdentity(identity), m_netId(m_netIdCounter++)
{
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

	Update(m_tickTimer);
	m_tickTimer -= m_sleepRate;
}


void NetSession::EncodeClientHandshake(ByteBuffer& buffer) 
{
	ByteBuffer content;

	// Build header
	Encode<Version>(content, m_engine->GetVersionNo());
	Encode<Version>(content, m_engine->GetGame()->GetVersionNo());
	Encode<uint16>(content, (uint16)NetRequestType::Connect);

	// TODO - Encode password
	// TODO - Encode username
}

NetRequestType NetSession::DecodeClientHandshake(ByteBuffer& inbuffer, ByteBuffer& outBuffer, uint16& outNetId) 
{
}

NetResponseCode NetSession::DecodeServerHandshake(ByteBuffer& buffer) 
{
}


void NetSession::EncodeEntityMessage(ByteBuffer& buffer, const SocketType& socketType, Entity* entity) 
{
	if ((!IsHost() && !entity->HasNetControl()) || !entity->IsNetSynced()) 
		return;


	// If entity has no id, it must be newly spawned
	if(entity->GetNetworkID() == 0)
	{
		// Was part of level
		if (entity->WasLoadedWithLevel())
		{
			entity->m_networkId = m_netIdCounter++; // Give a unique net id

			// TODO - Give net role

			Encode<uint8>(buffer, (uint8)NetEntityMethod::ControlOverride);
			Encode<uint16>(buffer, entity->m_networkId);
			Encode<uint16>(buffer, entity->m_networkOwnerId);
			Encode<uint32>(buffer, entity->GetInstanceID());
		}
		// Was dynamically spawned
		else
		{
			entity->m_networkId = m_netIdCounter++; // Give a unique net id

			// TODO - Give net role

			Encode<uint8>(buffer, (uint8)NetEntityMethod::Spawn);
			Encode<uint16>(buffer, entity->m_networkId);
			Encode<uint16>(buffer, entity->m_networkOwnerId);
			Encode<uint32>(buffer, entity->GetTypeID());
		}
	}
}

void NetSession::DecodeEntityMessage(const uint16& sourceNetId, ByteBuffer& buffer, const SocketType& socketType)
{
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
		LOG("Control %i to net %i", instanceId, netId);
		break;
	}

	case NetEntityMethod::Update:
		break;

	default:
		break;
	}
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