#include "Includes\Core\NetSession.h"
#include "Includes\Core\NetController.h"

#include "Includes\Core\Engine.h"
#include "Includes\Core\Game.h"
#include "Includes\Core\Level.h"
#include "Includes\Core\Logger.h"



NetSession::NetSession(Game* game, const NetIdentity identity) :
	m_game(game), m_netIdentity(identity)
{
	m_sessionNetId = 0;

	// 0 - Reservered as nullptr
	m_playerIdCounter = 1;
	m_objectNetIdCounter = 1; 
	m_actorNetIdCounter = 1;

	LOG("Using '%s' NetLayer", game->netLayerClass->GetName().c_str());
	m_netLayer = game->netLayerClass->New<NetLayer>();
	m_netLayer->Initialize(game, this);
}

NetSession::~NetSession()
{
	delete m_netLayer;
}


void NetSession::MainUpdate(const float& deltaTime) 
{
	m_tickTimer += deltaTime;
	if (m_tickTimer < m_sleepRate)
		return;

	if (m_tickTimer >= m_sleepRate * 5.0f)
		LOG_WARNING("Net update falling behind");

	PreNetUpdate();

	m_netLayer->OnNetTick(m_tickTimer);
	NetUpdate(m_tickTimer);

	PostNetUpdate();
	m_tickTimer = 0;
}

void NetSession::PreNetUpdate() 
{
	// Assign ids to any new objects
	for (OObject* object : GetGame()->GetActiveObjects())
	{
		if (!object->IsNetSynced())
			continue;

		if (object->HasNetControl())
			object->OnPreNetUpdate();

		// Assign new net id
		if (object->GetNetworkID() == 0 && IsHost())
		{
			object->m_networkId = NewObjectID();
			object->bFirstNetUpdate = true;
			object->UpdateRole(this);
			GetGame()->m_netObjectLookup[object->m_networkId] = object;
		}
	}


	// Assign ids to any new actors
	LLevel* level = GetGame()->GetCurrentLevel();
	if (level != nullptr)
		for (AActor* actor : level->GetActiveActors())
		{
			if (!actor->IsNetSynced())
				continue;

			if (actor->HasNetControl())
				actor->OnPreNetUpdate();

			// Assign new net id
			if (actor->GetNetworkID() == 0 && IsHost())
			{
				actor->m_networkId = NewActorID();
				actor->bFirstNetUpdate = true;
				actor->UpdateRole(this);
				level->m_netActorLookup[actor->m_networkId] = actor;
			}
		}
}

void NetSession::PostNetUpdate() 
{
	// Clear object net queues
	for (OObject* object : GetGame()->GetActiveObjects())
	{
		if (!object->IsNetSynced())
			continue;

		object->ClearQueuedNetData();
		object->bFirstNetUpdate = false;
	}


	// Clear actor net queues
	LLevel* level = GetGame()->GetCurrentLevel();
	if (level != nullptr)
		for (AActor* actor : level->GetActiveActors())
		{
			if (!actor->IsNetSynced())
				continue;

			actor->ClearQueuedNetData();
			actor->bFirstNetUpdate = false;
		}

	m_deletionQueue.clear();
}


void NetSession::OnNetObjectDestroy(const OObject* object)
{
	// Only worry about destroyed objects, if hosting
	if (!IsHost())
		return;

	// Queue needed info about deletion
	NetObjectDeletion info;
	info.bIsActor = dynamic_cast<const AActor*>(object) != nullptr;
	info.netId = object->GetNetworkID();
	m_deletionQueue.emplace_back(info);
}


void NetSession::EncodeNetObject(const OPlayerController* target, OObject* object, ByteBuffer& buffer, const SocketType& socketType)
{
	bool newConnection = (target != nullptr && target->bFirstNetUpdate);

	// New object
	if (object->bFirstNetUpdate || newConnection)
	{
		// Ignore if client (Server has to create entities)
		if (!IsHost())
			return;


		// Target already knows about itself
		if (object == target)
			return;

		// Must initialize on TCP
		if (socketType == UDP)
			return;


		// Encode new object information
		Encode<uint8>(buffer, (uint8)NetObjectMethod::New);	
		Encode<uint16>(buffer, object->GetNetworkID());	
		Encode<uint16>(buffer, object->GetNetworkOwnerID());
		Encode<uint16>(buffer, object->GetClass()->GetID());
		

		// Attempt to encode unique instance id for actors built in level load
		AActor* actor = dynamic_cast<AActor*>(object);
		if(actor == nullptr || !actor->WasSpawnedWithLevel())
			Encode<uint32>(buffer, 0);
		else
			Encode<uint32>(buffer, actor->GetInstanceID());


		// Encode all sync var values for initial sync
		const uint16 targetId = target == nullptr ? 0 : target->GetNetworkOwnerID();
		object->EncodeSyncVarRequests(targetId, buffer, socketType, true);

		return;
	}

	// Don't update if object has no data to send
	if (!object->HasQueuedNetData(socketType))
		return;
	Encode<uint8>(buffer, (uint8)NetObjectMethod::Update);

	// Encode sync vars and rpcs
	Encode<uint16>(buffer, object->GetNetworkID());
	const uint16 targetId = target == nullptr ? 0 : target->GetNetworkOwnerID();
	object->EncodeSyncVarRequests(targetId, buffer, socketType, false);
	object->EncodeRPCRequests(targetId, buffer, socketType);
}

void NetSession::DecodeNetObject(const OPlayerController* source, const bool& isActor, ByteBuffer& buffer, const SocketType& socketType, const bool& justCleanUp)
{
	uint8 rawMethod;
	Decode(buffer, rawMethod);
	NetObjectMethod method = (NetObjectMethod)rawMethod;

	switch (method)
	{
		// Ignore this
		case NetObjectMethod::Empty: 
			break; 


		// Create new object
		case NetObjectMethod::New:
		{
			// Decode and check values
			uint16 netId;
			uint16 ownerNetId;
			uint16 classId;
			uint32 instanceId;
			if (!Decode(buffer, netId) ||
				!Decode(buffer, ownerNetId) ||
				!Decode(buffer, classId) ||
				!Decode(buffer, instanceId))
			{
				LOG_ERROR("Received invalid data at NetObjectMethod::New");
				return;
			}

			// Cleared buffer, so leave
			if (justCleanUp)
				return;


			// Ignore if host
			if (IsHost())
			{
				LOG_ERROR("Received NetObjectMethod::New from client");
				return;
			}

			// Fetch and check class is correct
			const MClass* typeClass = isActor ? GetGame()->GetActorClass(classId) : GetGame()->GetObjectClass(classId);
			if (typeClass == nullptr)
			{
				LOG_ERROR("Received unidentified class for NetObjectMethod::New id:%i", classId);
				return;
			}


			if (isActor)
			{
				// This message was just syncing an actor which existed during level load
				if (instanceId != 0)
				{
					AActor* actor = GetGame()->GetCurrentLevel()->GetActorByInstance(instanceId);
					if (actor != nullptr)
					{
						actor->m_networkId = netId;
						actor->m_networkOwnerId = ownerNetId;
						actor->UpdateRole(this);

						const uint16 sourceId = source == nullptr ? 0 : source->GetNetworkOwnerID();
						actor->DecodeSyncVarRequests(sourceId, buffer, socketType, true);
						GetGame()->GetCurrentLevel()->m_netActorLookup[actor->m_networkId] = actor;
						actor->OnPostNetInitialize();
						return;
					}
					// If null, just create a new actor
				}

				// Create actor
				AActor* actor = typeClass->New<AActor>();
				actor->m_networkId = netId;
				actor->m_networkOwnerId = ownerNetId;
				actor->UpdateRole(this);

				const uint16 sourceId = source == nullptr ? 0 : source->GetNetworkOwnerID();
				GetGame()->GetCurrentLevel()->AddActor(dynamic_cast<AActor*>(actor));
				actor->DecodeSyncVarRequests(sourceId, buffer, socketType, true);
				actor->OnPostNetInitialize();
			}

			// Create object
			else
			{
				OObject* object = typeClass->New<OObject>();
				object->m_networkId = netId;
				object->m_networkOwnerId = ownerNetId;
				object->UpdateRole(this);

				const uint16 sourceId = source == nullptr ? 0 : source->GetNetworkOwnerID();
				GetGame()->AddObject(object);
				object->DecodeSyncVarRequests(sourceId, buffer, socketType, true);
				object->OnPostNetInitialize();
			}
			return;
		}


		case NetObjectMethod::Delete:
		{
			// Decode and check values
			uint16 netId;
			if (!Decode(buffer, netId))
			{
				LOG_ERROR("Received invalid data at NetObjectMethod::Delete");
				return;
			}
			OObject* object = isActor ? GetGame()->GetCurrentLevel()->GetActorByNetID(netId) : GetGame()->GetObjectByNetID(netId);
			OObject::Destroy(object);
			return;
		}


		case NetObjectMethod::Update:
		{
			// Decode and check values
			uint16 netId;
			if (!Decode(buffer, netId))
			{
				LOG_ERROR("Received invalid data at NetObjectMethod::Update");
				return;
			}

			OObject* object = isActor ? GetGame()->GetCurrentLevel()->GetActorByNetID(netId) : GetGame()->GetObjectByNetID(netId);
			if (object != nullptr)
			{
				const uint16 sourceId = source == nullptr ? 0 : source->GetNetworkOwnerID();
				object->DecodeSyncVarRequests(sourceId, buffer, socketType, false);
				object->DecodeRPCRequests(sourceId, buffer, socketType);
			}
			return;
		}
	}

	return;
}



void NetSession::NetEncode(const OPlayerController* target, ByteBuffer& buffer, const SocketType& socketType)
{
	// Encode level header
	LLevel* level = GetGame()->GetCurrentLevel();
	Encode<uint16>(buffer, level == nullptr ? 0 : level->GetInstanceID());


	uint16 messageCount = 0;
	ByteBuffer messageBuffer;


	// Encode any deleted net objects
	if(socketType == TCP)
		for (NetObjectDeletion& deletion : m_deletionQueue)
		{
			// Skip, if not in a level (Because we really don't care)
			if (deletion.bIsActor && level == nullptr)
				continue;

			Encode<uint8>(messageBuffer, (uint8)NetMessage::NetObjectMessage);
			Encode<uint8>(messageBuffer, deletion.bIsActor);
			Encode<uint8>(messageBuffer, (uint8)NetObjectMethod::Delete);
			Encode<uint16>(messageBuffer, deletion.netId);
			++messageCount;
		}

	
	// Encode objects
	for (OObject* object : GetGame()->GetActiveObjects())
		if (!object->IsDestroyed() && object->GetNetworkID() != 0 && (IsHost() || object->IsNetOwner()))
		{
			Encode<uint8>(messageBuffer, (uint8)NetMessage::NetObjectMessage); 
			Encode<uint8>(messageBuffer, 0); // 0 - Object

			const uint32 startSize = messageBuffer.Size();
			EncodeNetObject(target, object, messageBuffer, socketType);

			// Nothing changed, so just revert
			if (messageBuffer.Size() == startSize)
			{
				messageBuffer.Pop();
				messageBuffer.Pop();
			}
			else
				++messageCount;
		}

	// Encode actors
	if (level != nullptr)
		for (AActor* actor : level->GetActiveActors())
			if (!actor->IsDestroyed() && actor->GetNetworkID() != 0 && (IsHost() || actor->IsNetOwner()))
			{
				Encode<uint8>(messageBuffer, (uint8)NetMessage::NetObjectMessage);
				Encode<uint8>(messageBuffer, 1); // 1 - Actor

				const uint32 startSize = messageBuffer.Size();
				EncodeNetObject(target, actor, messageBuffer, socketType);

				// Nothing changed, so just revert
				if (messageBuffer.Size() == startSize)
				{
					messageBuffer.Pop();
					messageBuffer.Pop();
				}
				else
					++messageCount;
			}



	Encode<uint16>(buffer, messageCount);
	if(messageCount != 0)
		buffer.Push(messageBuffer.Data(), messageBuffer.Size());
}

void NetSession::NetDecode(const OPlayerController* source, ByteBuffer& buffer, const SocketType& socketType)
{
	uint16 levelId;
	uint16 messageCount;
	Decode<uint16>(buffer, levelId);
	Decode<uint16>(buffer, messageCount);

	LLevel* level = GetGame()->GetCurrentLevel();
	const bool levelIsValid = (level != nullptr && levelId == level->GetInstanceID());


	// Decode all object messages
	for (uint32 i = 0; i < messageCount; ++i)
	{
		uint8 rawMsgType;
		if (!Decode(buffer, rawMsgType))
			return;
		NetMessage msgType = (NetMessage)rawMsgType;
		
		switch (msgType)
		{
			case NetMessage::Nothing:
				break;

			// Decode any net object information
			case NetMessage::NetObjectMessage:
			{
				uint8 isActor = 0;
				if (Decode(buffer, isActor))
				{
					const bool justCleanBuffer = isActor && !levelIsValid; // i.e. Ignore this message
					DecodeNetObject(source, isActor, buffer, socketType, justCleanBuffer);
				}
				break;
			}

			default:
				break;
		}
	}
}