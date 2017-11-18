#pragma once
#include "Core\Core-Common.h"
#include "BPlayerController.h"


/**
* Level controller base for all BomberBoy gamemodes
*/
class ABLevelControllerBase : public ALevelController
{
	CLASS_BODY()
private:
	std::vector<OBPlayerController*> activePlayers;
public:
	ABLevelControllerBase();


	virtual void OnPlayerConnect(OPlayerController* player, const bool& newConnection) override;
	virtual void OnPlayerDisconnect(OPlayerController* player) override;


	/**
	* Getters & Setters
	*/
public:
	inline const std::vector<OBPlayerController*> GetActivePlayers() const { return activePlayers; }
};

