#pragma once
#include "NetSession.h"


/**
* Represents a session that this process is currently the host of
*/
class NetHostSession : public NetSession
{
private:
	std::map<NetIdentity, NetClient*> m_clientLookup;
	std::vector<NetClient*> m_clients;
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

private:

	/**
	* Fetch a specific client for a given identity
	* @param identity				The identity we're looking for
	* @returns Desired client or nullptr if not found/created
	*/
	NetClient* GetClient(const NetIdentity& identity);

	/**
	* Attempt to accept of deny the client
	* @param client				The client we want to perform the handshake on
	* @param packet				The initial handshake packet passed
	* @returns The response code that was sent to the client
	*/
	NetResponseCode VerifyHandshake(NetClient* client, RawNetPacket& packet, NetSocket& socket);

	/**
	* Forward a packet to a client
	* @param packet				The net packet received
	* @param socket				The socket that this data was received from
	*/
	void ForwardToClient(RawNetPacket& packet, NetSocket& socket);
};