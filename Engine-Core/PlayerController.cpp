#include "Includes\Core\PlayerController.h"

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


void OPlayerController::OnBegin() 
{
	Super::OnBegin();
	m_playerName = "Player_" + std::to_string(GetNetworkOwnerID());
	LOG("Player(%i) '%s' has joined.", GetNetworkOwnerID(), m_playerName.c_str());
}

void OPlayerController::OnDestroy() 
{
	Super::OnDestroy();
	LOG("Player(%i) '%s' has left.", GetNetworkOwnerID(), m_playerName.c_str());
}


void OPlayerController::SetPlayerName(string name) 
{
	m_playerName = name;
}