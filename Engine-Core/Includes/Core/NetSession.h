#pragma once
#include "Common.h"
#include "NetSocketTcp.h"
#include "NetSocketUdp.h"


/**
* Represents the current connection between player(s) and server
*/
class CORE_API NetSession
{
private:
	NetSocketTcp m_TcpSocket;
	NetSocketUdp m_UdpSocket;

	const uint32 tickRate;
	const float desiredTickRate = 1.0f / (float)tickRate;
	float m_tickTimer = 0.0f;

	bool bIsHost = false;
	bool bIsConnected = false;

	NetSession();
public:
	~NetSession();

	/**
	* Attempt to host a new game session over this network identity
	* @param host			The ip:port to host over
	* @returns New session that has been started or nullptr if failed to open session
	*/
	static NetSession* HostSession(const NetIdentity& host);
	
	/**
	* Attempt to connect to a remotely hosted game session at this network identity
	* @param remote			The ip:port to connect to
	* @returns New session that has been connected to or nullptr if failed to join session
	*/
	static NetSession* JoinSession(const NetIdentity& remote);

	/**
	* Callback from engine for every tick by main
	* @param engine			The engine + game to update using
	* @param deltaTime		Time since last update (In seconds)
	*/
	void Update(class Engine* engine, const float& deltaTime);


	/**
	* Getters and setters
	*/
public:
	inline const bool& IsHost() const { return bIsHost; }
	inline const bool& IsRemote() const { return bIsHost; }
};

