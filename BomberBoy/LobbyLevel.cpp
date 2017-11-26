#include "LobbyLevel.h"

#include "LobbyController.h"
#include "LobbyHUD.h"

CLASS_SOURCE(LLobbyLevel)


LLobbyLevel::LLobbyLevel()
{
	levelControllerClass = ALobbyController::StaticClass();
	hudClass = ALobbyHUD::StaticClass();
}