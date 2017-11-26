#include "LobbyController.h"

#include "LobbyHUD.h"


CLASS_SOURCE(ALobbyController)


ALobbyController::ALobbyController()
{
}

void ALobbyController::OnPlayerConnect(OPlayerController* player, const bool& newConnection) 
{
	Super::OnPlayerConnect(player, newConnection);


	OBPlayerController* bplayer = dynamic_cast<OBPlayerController*>(player);
	ALobbyHUD* hud = dynamic_cast<ALobbyHUD*>(GetLevel()->GetHUD());

	if (bplayer != nullptr && hud != nullptr)
		hud->OnPlayerConnect(bplayer);

}

void ALobbyController::OnPlayerDisconnect(OPlayerController* player) 
{
	Super::OnPlayerDisconnect(player);

	OBPlayerController* bplayer = dynamic_cast<OBPlayerController*>(player);
	ALobbyHUD* hud = dynamic_cast<ALobbyHUD*>(GetLevel()->GetHUD());

	if (bplayer != nullptr && hud != nullptr)
		hud->OnPlayerDisconnect(bplayer);
}