#pragma once
#include "NetSession.h"
#include "NetPlayer.h"


/**
* Represents a session that this process is currently the host of
*/
class NetHostSession : public NetSession
{
private:
	uint32 m_playerIdCounter = 1;
	std::map<const NetIdentity, NetPlayer*> m_playerLookup;
	std::vector<NetPlayer*> m_players;
	uint16 maxPlayerCount = 10;

public:
	NetHostSession(const Engine* engine, const NetIdentity identity);
	virtual ~NetHostSession();

	/**
	* Attempt to start up the session at/on this identity
	* @returns If setup correctly
	*/
	virtual bool Start();

	/**
	* Called from handle update, at desired tickrate (Previously set)
	* @param engine			The engine + game to update using
	* @param deltaTime		Time since last update (In seconds)
	*/
	virtual void Update(const float& deltaTime);

protected:
	/**
	* Encode any relevant information to be sent out this net update
	* @param buffer			Where to store all information
	* @param socketType		The socket type this content will be sent over
	*/
	virtual void NetEncode(ByteBuffer& buffer, const SocketType& socketType);

private:
	/**
	* Attempt to accept or deny the player connection to the server
	* @param player				The player we want to perform the handshake on
	* @param packet				The initial handshake packet passed
	* @returns The response code that was sent to the client
	*/
	NetResponseCode VerifyHandshake(NetPlayer* player, RawNetPacket& packet, NetSocket& socket);

	/**
	* Fetch a player's network id from their connecting identity
	* @param identity			Where the player is connecting from
	* @param outPlayer			Where the player will be stored
	* @returns If id is succesfully fetched
	*/
	bool GetPlayerFromIdentity(const NetIdentity& identity, NetPlayer*& outPlayer) const;
};