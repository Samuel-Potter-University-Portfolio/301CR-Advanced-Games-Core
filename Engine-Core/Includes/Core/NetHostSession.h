#pragma once
#include "NetSession.h"


/**
* Holds information about an incoming player connection
*/
struct NetPlayerConnection 
{
	NetIdentity			identity;
	OPlayerController*	controller = nullptr;
	float				inactivityTimer = 0;

	uint16				currentLevelClass = 0;		// The class id of the level that this client is on
	uint16				currentLevelInstance = 0;	// The instance id of the level that this client is on
	bool				bJustLoadedLevel = false;	// This client has just loaded this level this update
	
	enum State : uint8
	{
		Intialized,			// Only just created
		Waiting,			// Waiting for something to internally finish (Probably NetLayer upstream logic)
		Connected,			// Fully connected into the game
	} state = State::Intialized;
};


/**
* Represents a session that this process is currently the host of
*/
class NetHostSession : public NetSession
{
private:
	std::map<const NetIdentity, NetPlayerConnection*> m_connectionLookup;
	const float m_maxInactivityTime = 15.0f;

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
	* Decode the client handshake that has been received by server
	* @param source				The player that is currently trying to connect
	* @param inbuffer			Where to read the handshake from
	* @param outPlayer			Where to store the resulting player controller
	* @returns The response code that will be sent to the client
	*/
	NetResponseCode DecodeHandshake(const NetIdentity& source, ByteBuffer& inBuffer, OPlayerController*& outPlayer);
	/**
	* Encode a handshake response to be returned
	* @param code				The status code of the response to write
	* @param outBuffer			Where to write the response
	* @param player				The player controller (If code is Accepted, null otherwise)
	*/
	void EncodeHandshakeResponse(const NetResponseCode& code, ByteBuffer& outBuffer, OPlayerController* player);

	/**
	* Fetch a player's network id from their connecting identity
	* @param identity			Where the player is connecting from
	* @param outPlayer			Where the player will be stored
	* @returns If id is succesfully fetched
	*/
	bool GetPlayerFromIdentity(const NetIdentity& identity, NetPlayerConnection*& outPlayer) const;


	/**
	* Getters & Setters
	*/
public:
	inline uint32 GetPlayerCount() const
	{
#ifdef BUILD_CLIENT
		return m_connectionLookup.size() + 1; // Include self (Only calls this if host)
#else
		return m_connectionLookup.size(); 
#endif
	}
};