#pragma once
#include "Actor.h"
#include "PlayerController.h"


/**
* Level controller will be spawned for each level and provides specific callbacks for some game events
* This should also be used to keep track of any game mode data
* -NOTE: Is synced, so gamemode data should be stored here
*/
class CORE_API ALevelController : public AActor
{
	CLASS_BODY()
public:
	ALevelController();

	/**
	* Callback for when a player connects to the level
	* @param player				The player who has connected
	* @param newConnection		Is this player a new player, or were they connected previously (Last level)
	*/
	virtual void OnPlayerConnect(OPlayerController* player, const bool& newConnection);

	/**
	* Callback for when a player disconnects
	* @param player				The player who has disconnected
	*/
	virtual void OnPlayerDisconnect(OPlayerController* player);
};

