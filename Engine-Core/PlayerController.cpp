#include "Includes\Core\PlayerController.h"
#include <Windows.h>


CLASS_SOURCE(OPlayerController, CORE_API)


OPlayerController::OPlayerController()
{
	bIsNetSynced = true;
}

bool OPlayerController::RegisterRPCs(const char* func, RPCInfo& outInfo) const
{
	RPC_INDEX_HEADER(func, outInfo);
	RPC_INDEX(TCP, RPCCallingMode::Host, SetPlayerName);
	return false;
}

bool OPlayerController::ExecuteRPC(uint16& id, ByteBuffer& params)
{
	RPC_EXEC_HEADER(id, params);
	RPC_EXEC_OneParam(SetPlayerName, string);
	return false;
}

void OPlayerController::RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode) 
{
	SYNCVAR_INDEX_HEADER(outQueue, socketType, index, trackIndex, forceEncode);
	SYNCVAR_INDEX(TCP, SyncVarMode::OnChange, string, m_playerName);
}

bool OPlayerController::ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks)
{
	SYNCVAR_EXEC_HEADER(id, value, skipCallbacks);
	if (__TEMP_ID == 0) 
	{ 
		const bool decoded = Decode(__TEMP_BUFFER, m_playerName);
		if (!decoded) return false; 
			if (!__TEMP_SKIP_CALLBACKS) OnNameChange();
				return true; 
	} 
	else 
		--__TEMP_ID;

	//SYNCVAR_EXEC_Callback(m_playerName, OnNameChange);
	return false;
}


void OPlayerController::OnBegin() 
{
	Super::OnBegin();

	// Set player name to system user name
	if (IsNetOwner() && m_playerName.empty())
	{
		TCHAR name[STR_MAX_ENCODE_LEN];
		DWORD count = STR_MAX_ENCODE_LEN;
		const string idPart = (GetNetworkOwnerID() == 0 ? "" : "(" + std::to_string(GetNetworkOwnerID()) + ")");
		if(GetUserName(name, &count))
			m_playerName = name + idPart;
		else
			m_playerName = "Player" + idPart;
	}

	LOG("Player(%i) '%s' has joined.", GetNetworkOwnerID(), m_playerName.c_str());
}

void OPlayerController::OnDestroy() 
{
	Super::OnDestroy();
	LOG("Player(%i) '%s' has left.", GetNetworkOwnerID(), m_playerName.c_str());
}


void OPlayerController::SetPlayerName(const string& name)
{
	m_playerName = name;
	OnNameChange();
}

void OPlayerController::OnNameChange() 
{
	LOG("Player(%i) renamed to '%s'", GetNetworkOwnerID(), m_playerName.c_str());
}