#pragma once
#include "NetSession.h"


/**
* Holds information about an incoming player connection
*/
struct NetPlayerConnection 
{
	NetIdentity			identity;
	OPlayerController*	controller = nullptr;
};


/**
* Represents a session that this process is currently the host of
*/
class NetHostSession : public NetSession
{
private:
	std::map<const NetIdentity, NetPlayerConnection> m_connectionLookup;

public:
	NetHostSession(Game* game, const NetIdentity identity);
	virtual ~NetHostSession();

	/**
	* Attempt to start up the session at/on this identity
	* @returns If setup correctly
	*/
	virtual bool Start() override;

	/**
	* Callback every time there should be a network update (IO to be polled/pushed)
	* @param deltaTime		Time since last update (In seconds)
	*/
	virtual void NetUpdate(const float& deltaTime) override;

private:

	/**
	* Fetch a player's network id from their connecting identity
	* @param identity			Where the player is connecting from
	* @param outPlayer			Where the player will be stored
	* @returns If id is succesfully fetched
	*/
	bool GetPlayerFromIdentity(const NetIdentity& identity, NetPlayerConnection& outPlayer) const;
};