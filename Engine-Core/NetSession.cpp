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
}

NetSession::~NetSession()
{
}


void NetSession::MainUpdate(const float& deltaTime) 
{
	m_tickTimer += deltaTime;
	if (m_tickTimer < m_sleepRate)
		return;

	if (m_tickTimer >= m_sleepRate * 5.0f)
		LOG_WARNING("Net update falling behind");

	PreNetUpdate();
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


void NetSession::EncodeHandshake_ClientToServer(ByteBuffer& outBuffer) 
{
	// Version numbers
	Encode(outBuffer, GetGame()->GetEngine()->GetVersionNo());
	Encode(outBuffer, GetGame()->GetVersionNo());

	// Request type
	Encode<uint16>(outBuffer, (uint16)NetRequestType::Connect);

	// Encode player name
	auto playerList = GetGame()->GetActiveObjects<OPlayerController>();
	if (playerList.size() != 0)
		Encode<string>(outBuffer, playerList[0]->GetPlayerName());
	else
		Encode<const char*>(outBuffer, "__PLAYER__");

	// TODO - 200 Content
	//   - Password
}

NetResponseCode NetSession::DecodeHandshake_ClientToServer(ByteBuffer& inBuffer, ByteBuffer& outBuffer, OPlayerController*& outPlayer)
{
	NetRequestType requestType;

	{
		Version engineVersion, gameVersion;
		uint16 rawRequestType;

		// Invalid header
		if (!Decode<Version>(inBuffer, engineVersion) ||
			!Decode<Version>(inBuffer, gameVersion) ||
			!Decode<uint16>(inBuffer, rawRequestType))
		{
			Encode<uint16>(outBuffer, (uint16)NetResponseCode::BadRequest);
			return NetResponseCode::BadRequest;
		}

		// Missmatching versions
		if (GetGame()->GetEngine()->GetVersionNo() != engineVersion || GetGame()->GetVersionNo() != gameVersion)
		{
			Encode<uint16>(outBuffer, (uint16)NetResponseCode::BadVersions);
			return NetResponseCode::BadVersions;
		}

		// TODO - Ban checks
		// TODO - Whitelist checks

		requestType = (NetRequestType)rawRequestType;
	}


	switch (requestType)
	{
		// Acknowledge by pinging back
		case NetRequestType::Ping:
		{
			Encode<uint16>(outBuffer, (uint16)NetResponseCode::Responded);
			return NetResponseCode::Responded;
		}


		// Attempt to accept the player
		case NetRequestType::Connect:
		{
			string playerName;
			string password; // TODO

			// Invalid information provided
			if (!Decode<string>(inBuffer, playerName))
			{
				Encode<uint16>(outBuffer, (uint16)NetResponseCode::BadRequest);
				return NetResponseCode::BadRequest;
			}
			

			// Server is full
			if (m_playerControllers.size() >= GetMaxPlayerCount())
			{
				Encode<uint16>(outBuffer, (uint16)NetResponseCode::ServerFull);
				return NetResponseCode::ServerFull;
			}

			// TODO - Password checks


			// Create new player
			outPlayer = GetGame()->playerControllerClass->New<OPlayerController>();
			outPlayer->m_playerName = playerName;
			outPlayer->m_networkOwnerId = NewPlayerID();
			outPlayer->m_networkId = NewObjectID();
			outPlayer->bFirstNetUpdate = true;
			outPlayer->UpdateRole(this);
			m_playerControllers.emplace_back(outPlayer);
			GetGame()->AddObject(outPlayer);
			outPlayer->OnPostNetInitialize();


			// Encode new player connection information
			Encode<uint16>(outBuffer, (uint16)NetResponseCode::Accepted);
			Encode<uint16>(outBuffer, outPlayer->m_networkOwnerId);
			Encode<uint16>(outBuffer, outPlayer->m_networkId);

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
		
			outPlayer->EncodeSyncVarRequests(outPlayer->m_networkOwnerId, outBuffer, TCP, true);
			return NetResponseCode::Accepted;
		}


		// Return server information
		case NetRequestType::Query:
		{
			Encode<uint16>(outBuffer, (uint16)NetResponseCode::Responded);
			Encode<uint16>(outBuffer, m_playerControllers.size());		// Players connected 
			Encode<uint16>(outBuffer, m_maxPlayerCount);				// Player limit
			Encode<string>(outBuffer, "Unnamed Server");				// TODO - Server name
			Encode<uint8>(outBuffer, 0);								// TODO - Bitflags
			return NetResponseCode::Responded;
		}

	}

	return NetResponseCode::Unknown;
}

NetResponseCode NetSession::DecodeHandshake_ServerToClient(ByteBuffer& inBuffer, OPlayerController*& outPlayer)
{
	uint16 rawResponse;
	if(!Decode<uint16>(inBuffer, rawResponse))
		return NetResponseCode::Unknown;

	const NetResponseCode response = (NetResponseCode)rawResponse;

	// Retrieve connection info
	if (response == NetResponseCode::Accepted)
	{
		uint16 netId;
		uint16 controllerId;
		uint16 levelClassId;
		uint32 levelInstanceId;

		// Decode information
		if (!Decode<uint16>(inBuffer, netId) ||
			!Decode<uint16>(inBuffer, controllerId) ||
			!Decode<uint16>(inBuffer, levelClassId) ||
			!Decode<uint32>(inBuffer, levelInstanceId)
		)
		{
			LOG_ERROR("Server's response to handshake is unparsable.");
			return NetResponseCode::BadRequest;
		}


		// Use an existing controller
		auto playerList = GetGame()->GetActiveObjects<OPlayerController>();
		if (playerList.size() >= 1)
		{
			outPlayer = playerList[0];

			// Cleaup any other controllers that may exist for some reason
			for (uint32 i = 1; i < playerList.size(); ++i)
				OObject::Destroy(playerList[i]);

			// As we are reusing this controller, let level cleanup
			LLevel* level = GetGame()->GetCurrentLevel();
			if (level != nullptr)
				level->GetLevelController()->OnPlayerDisconnect(outPlayer);
		}
		else if (playerList.size() == 0)
			outPlayer = GetGame()->playerControllerClass->New<OPlayerController>();
		

		// Setup new player controller
		m_sessionNetId = netId;
		outPlayer->m_networkOwnerId = netId;
		outPlayer->m_networkId = controllerId;
		outPlayer->bFirstNetUpdate = true;
		outPlayer->UpdateRole(this);
		outPlayer->DecodeSyncVarRequests(netId, inBuffer, TCP, true);
		GetGame()->AddObject(outPlayer);
		outPlayer->OnPostNetInitialize();



		// Switch to desired level
		LLevel::s_instanceCounter = levelInstanceId - 100000; // Make sure is decently away from server's instance counter
		if (!GetGame()->SwitchLevel(levelClassId)) 
		{
			LOG_ERROR("Unable to switch to server requested level");
			return NetResponseCode::BadRequest;
		}
		else 
		{
			// Set level's instance id (Works because levels don't load asynchronously)
			LLevel* level = GetGame()->GetCurrentLevel();
			level->m_instanceId = levelInstanceId;
		}
	}

	return response;
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
				actor->DecodeSyncVarRequests(sourceId, buffer, socketType, true);
				GetGame()->GetCurrentLevel()->AddActor(dynamic_cast<AActor*>(actor));
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
				object->DecodeSyncVarRequests(sourceId, buffer, socketType, true);
				GetGame()->AddObject(object);
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


	// Decode all  object messages
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