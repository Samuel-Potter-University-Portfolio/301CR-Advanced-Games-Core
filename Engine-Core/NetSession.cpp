#include "Includes\Core\NetSession.h"
#include "Includes\Core\NetController.h"
#include "Includes\Core\NetHostSession.h"

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

	m_netLayer = nullptr;

	m_sessionName = game->GetName() + " Server";
}

NetSession::~NetSession()
{
	delete m_netLayer;
}

void NetSession::SetupLayer(SubClassOf<NetLayer> layerType, ConfigLayer configLayer) 
{
	LOG("Using '%s' as NetLayer", layerType->GetName().c_str());
	m_netLayer = layerType->New<NetLayer>();
	m_netLayer->Initialize(GetGame(), this, configLayer);
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
		if (!object->IsNetSynced() || object->IsDestroyed())
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
			if (!actor->IsNetSynced() || actor->IsDestroyed())
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


void NetSession::EncodeNetObject(NetPlayerConnection* target, OObject* object, ByteBuffer& buffer, const SocketType& socketType, const bool& encodeAsNew)
{
	const uint16 targetId = target == nullptr ? 0 : target->controller->GetNetworkOwnerID();

	// New object
	if (object->bFirstNetUpdate || encodeAsNew)
	{
		// Ignore if client (Server has to create all objects)
		if (!IsHost())
			return;

		// Must initialize over TCP
		if (socketType == UDP)
			return;

		// Ignore, if trying to tell client about it's own controller
		if (target != nullptr && object == target->controller)
			return;

		// Encode new object information
		Encode<uint8>(buffer, (uint8)NetObjectMethod::New);
		Encode<uint16>(buffer, object->GetNetworkID());
		Encode<uint16>(buffer, object->GetNetworkOwnerID());
		Encode<uint16>(buffer, object->GetClass()->GetID());


		// Attempt to encode unique instance id for actors built in level load
		AActor* actor = dynamic_cast<AActor*>(object);
		if (actor == nullptr || !actor->WasSpawnedWithLevel())
			Encode<uint32>(buffer, 0);
		else
			Encode<uint32>(buffer, actor->GetInstanceID());


		// Encode all sync var values for initial sync
		object->EncodeSyncVarRequests(targetId, buffer, socketType, true);
		return;
	}

	// Don't update if object has no data to send
	if (!object->HasQueuedNetData(socketType))
		return;
	Encode<uint8>(buffer, (uint8)NetObjectMethod::Update);

	// Encode sync vars and rpcs
	Encode<uint16>(buffer, object->GetNetworkID());
	object->EncodeSyncVarRequests(targetId, buffer, socketType, false);
	object->EncodeRPCRequests(targetId, buffer, socketType);
}

void NetSession::DecodeNetObject(NetPlayerConnection* source, const bool& isActor, ByteBuffer& buffer, const SocketType& socketType)
{
	const uint16 sourceId = source == nullptr ? 0 : source->controller->GetNetworkOwnerID();

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

			// Only host can issue object deletes
			if (IsHost())
			{
				LOG_ERROR("Received NetObjectMethod::Delete from client");
				return;
			}


			// Get object
			OObject* object = nullptr;
			if (isActor)
			{
				LLevel* level = GetGame()->GetCurrentLevel();
				if (level != nullptr)
					object = level->GetActorByNetID(netId);

			}
			else
				object = GetGame()->GetObjectByNetID(netId);;

			if (object != nullptr)
				OObject::Destroy(object);
			else
				LOG_WARNING("Unable to find object to perform NetObjectMethod::Delete desync incoming.");
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

			// Get object
			OObject* object = nullptr;
			if (isActor)
			{
				LLevel* level = GetGame()->GetCurrentLevel();
				if (level != nullptr)
					object = level->GetActorByNetID(netId);

			}
			else
				object = GetGame()->GetObjectByNetID(netId);;


			// Update object
			if (object != nullptr)
			{
				object->DecodeSyncVarRequests(sourceId, buffer, socketType, false);
				object->DecodeRPCRequests(sourceId, buffer, socketType);
			}
			else
				LOG_WARNING("Unable to find object to perform NetObjectMethod::Update desync incoming.");
			return;
		}
	};
}


void NetSession::EncodeNetUpdate(NetPlayerConnection* target, ByteBuffer& buffer, const SocketType& socketType)
{
	const bool firstUpdate = target != nullptr && target->controller->bFirstNetUpdate;

	uint16 messageCount = 0;
	ByteBuffer messageBuffer;


	// Encode all objects
	{
		for (OObject* object : GetGame()->GetActiveObjects())
			if (!object->IsDestroyed() && object->GetNetworkID() != 0 && (IsHost() || object->IsNetOwner()))
			{
				const uint32 startSize = messageBuffer.Size();
				EncodeNetObject(target, object, messageBuffer, socketType, firstUpdate);

				// Only count message, if something was encoded
				if (messageBuffer.Size() != startSize)
					++messageCount;
			}


		// Encode deleted objects
		if (socketType == TCP)
			for (NetObjectDeletion& deletion : m_deletionQueue)
			{
				// Ignore actors
				if (deletion.bIsActor)
					continue;

				Encode<uint8>(messageBuffer, (uint8)NetObjectMethod::Delete);
				Encode<uint16>(messageBuffer, deletion.netId);
				++messageCount;
			}
	}

	// Add messages to buffer
	Encode<uint16>(buffer, messageCount);
	if(messageCount != 0)
		buffer.Push(messageBuffer.Data(), messageBuffer.Size());



	// Encode level information
	messageCount = 0;
	messageBuffer.Clear();
	LLevel* level = GetGame()->GetCurrentLevel();
	const bool firstLevelUpdate = target != nullptr && target->bJustLoadedLevel;


	// No level loaded, so fill with empty info
	if (level == nullptr)
	{
		Encode<uint16>(buffer, 0);
		Encode<uint16>(buffer, 0);

		Encode<uint16>(buffer, 0); // 0 Actor messages, as not loaded into any level!
		return;
	}
	else
	{
		const uint16 levelClass = level->GetClass()->GetID();
		const uint16 levelInstance = level->GetInstanceID();

		Encode<uint16>(buffer, levelClass);
		Encode<uint16>(buffer, levelInstance);

		// Check client is on same level
		if (target != nullptr)
		{
			if (target->currentLevelClass != levelClass || target->currentLevelInstance != levelInstance)
			{
				// 0 Actor messages, as not loaded into correct level
				Encode<uint16>(buffer, 0);
				return;
			}
		}
	}



	// Encode all actors
	{
		for (AActor* actor : level->GetActiveActors())
			if (!actor->IsDestroyed() && actor->GetNetworkID() != 0 && (IsHost() || actor->IsNetOwner()))
			{
				const uint32 startSize = messageBuffer.Size();
				EncodeNetObject(target, actor, messageBuffer, socketType, firstLevelUpdate);

				// Only count message, if something was encoded
				if (messageBuffer.Size() != startSize)
					++messageCount;
			}

		
		// Encode deleted actors
		if (socketType == TCP && IsHost())
			for (NetObjectDeletion& deletion : m_deletionQueue)
			{
				// Ignore objects
				if (!deletion.bIsActor)
					continue;

				Encode<uint8>(messageBuffer, (uint8)NetObjectMethod::Delete);
				Encode<uint16>(messageBuffer, deletion.netId);
				++messageCount;
			}
	}


	// Add messages to buffer
	Encode<uint16>(buffer, messageCount);
	if (messageCount != 0)
		buffer.Push(messageBuffer.Data(), messageBuffer.Size());
}

void NetSession::DecodeNetUpdate(NetPlayerConnection* source, ByteBuffer& buffer, const SocketType& socketType)
{
	uint16 messageCount = 0;
	if (!Decode<uint16>(buffer, messageCount)) return;


	// Decode object messages
	for (uint32 i = 0; i < messageCount; ++i)
		DecodeNetObject(source, false, buffer, socketType);
	


	// Fetch level information
	LLevel* level = GetGame()->GetCurrentLevel();
	uint16 levelClass;
	uint16 levelInstance;

	if (!Decode<uint16>(buffer, levelClass) || !Decode<uint16>(buffer, levelInstance))
		return;


	// Ignore call/Update player level state based on what level they are currently on
	if (IsHost())
	{
		// Not loaded into any level anyways
		if (level == nullptr)
			return;

		// User is not connect to correct level
		if (level->GetInstanceID() != levelInstance || level->GetClass()->GetID() != levelClass)
			return;

		// User has just connected to this level
		if (source->currentLevelInstance != levelInstance || source->currentLevelClass != levelClass)
		{
			source->currentLevelInstance = levelInstance;
			source->currentLevelClass = levelClass;
			source->bJustLoadedLevel = true;
		}

		// User has only just loaded level, so needs to receive input before can do any actor calls
		if (source->bJustLoadedLevel)
			return;
	}

	// Attempt to switch to correct level, if not already
	else 
	{
		if (level == nullptr || level->GetInstanceID() != levelInstance || level->GetClass()->GetID() != levelClass)
		{
			// Attempt to switch to level
			LLevel::s_instanceCounter = 10000 - levelInstance; // Just to make sure we are definately on wrong level, if we switch
			LLevel* newLevel;
			if (!GetGame()->HasPendingLevelSwitch())
			{
				if (GetGame()->SwitchLevel(levelClass, newLevel))
					newLevel->m_instanceId = levelInstance;
				else
					LOG("Failed to switch to requested level { class:%i instance:%i }", levelClass, levelInstance);
			}
			return;
		}
	}



	messageCount = 0;
	if (!Decode<uint16>(buffer, messageCount)) return;


	// Decode actor messages
	for (uint32 i = 0; i < messageCount; ++i)
		DecodeNetObject(source, true, buffer, socketType);
}