#include "Includes\Core\LevelController.h"


CLASS_SOURCE(ALevelController, CORE_API)


ALevelController::ALevelController()
{
	bIsNetSynced = true;
}

void ALevelController::OnPlayerConnect(OPlayerController* player, const bool& newConnection) 
{
	if(newConnection)
		LOG("Player(%i) '%s' has joined.", player->GetNetworkOwnerID(), player->GetPlayerName().c_str());
}

void ALevelController::OnPlayerDisconnect(OPlayerController* player) 
{
	LOG("Player(%i) '%s' has left.", player->GetNetworkOwnerID(), player->GetPlayerName().c_str());
}