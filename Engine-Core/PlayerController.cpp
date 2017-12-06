#include "Includes\Core\PlayerController.h"
#include <Windows.h>

#include "Includes\Core\Game.h"
#include "Includes\Core\LevelController.h"


CLASS_SOURCE(OPlayerController, CORE_API)


OPlayerController::OPlayerController()
{
	bIsNetSynced = true;
}

bool OPlayerController::RegisterRPCs(const char* func, RPCInfo& outInfo) const
{
	RPC_INDEX_HEADER(func, outInfo);
	return false;
}

bool OPlayerController::ExecuteRPC(uint16& id, ByteBuffer& params)
{
	RPC_EXEC_HEADER(id, params);
	return false;
}

void OPlayerController::RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode) 
{
	SYNCVAR_INDEX_HEADER(outQueue, socketType, index, trackIndex, forceEncode);
	SYNCVAR_INDEX(TCP, string, m_playerName);
}

bool OPlayerController::ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks)
{
	SYNCVAR_EXEC_HEADER(id, value, skipCallbacks);
	SYNCVAR_EXEC_Callback(m_playerName, OnNameChange);
	return false;
}


void OPlayerController::OnBegin() 
{
	Super::OnBegin();

	// Set player name
	if (m_playerName.empty())
	{
		// Use PC name as player's name
		string playerName;
		TCHAR name[STR_MAX_ENCODE_LEN];
		DWORD count = STR_MAX_ENCODE_LEN;
		srand(time(nullptr));
		uint32 id = rand() % 10000;
		if (GetUserName(name, &count))
			m_playerName = string(name) + "_" + std::to_string(id);
		else
			m_playerName = "Player_" + std::to_string(id);
	}


	// Call player connect callback
	LLevel* level = GetGame()->GetCurrentLevel();
	if (level != nullptr && level->GetLevelController() != nullptr)
		level->GetLevelController()->OnPlayerConnect(this, true);

}

void OPlayerController::OnDestroy() 
{
	Super::OnDestroy();


	// Call player disconnect callback
	LLevel* level = GetGame()->GetCurrentLevel();
	if (level != nullptr && level->GetLevelController() != nullptr)
		level->GetLevelController()->OnPlayerDisconnect(this);

}


void OPlayerController::SetPlayerName(const string& name)
{
	m_playerName = name;
}

void OPlayerController::OnNameChange() 
{
	LOG("Player(%i) renamed to '%s'", GetNetworkOwnerID(), m_playerName.c_str());
}