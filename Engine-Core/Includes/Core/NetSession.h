#pragma once
#include "Common.h"
#include "NetClient.h"

#include "NetSocketTcp.h"
#include "NetSocketUdp.h"


class Engine;


/**
* What sort of requests can be performed during a handshake
*/
enum class NetRequestType : uint16
{
	Ping		= 0,
	Connect		= 200,
	Query		= 201
};


/**
* Response codes used in the initial handshake
*/
enum class NetResponseCode : uint16
{
	Unknown			= 0,

	Accepted		= 200,
	Responded		= 201,

	BadRequest		= 400,
	Banned			= 401,
	BadPassword		= 403,
	BadVersions		= 426,

	ServerFull		= 512
};


/**
* What this current packet is trying to do
*/
enum class NetPacketType : uint8
{
	Nothing					= 0,
	EntitySpawn				= 1,
	EntityDespawn			= 2,
	EntityControlOverride	= 3,
	EntityUpdate			= 4,
};



/**
* Abstract class represents the current connection between player(s) and a server
*/
class CORE_API NetSession
{
private:
	const NetIdentity m_netIdentity;

	uint32 m_tickRate = 20;
	float m_sleepRate = 1.0f / (float)m_tickRate;
	float m_tickTimer = 0.0f;

protected:
	NetSocketTcp m_TcpSocket;
	NetSocketUdp m_UdpSocket;
	const Engine* m_engine;

	bool bIsHost = false;
	bool bIsConnected = false;

public:
	NetSession(const Engine* engine, const NetIdentity identity);
	virtual ~NetSession();

	/**
	* Attempt to start up the session at/on this identity
	* @returns If setup correctly
	*/
	virtual bool Start() = 0;

	/**
	* Callback from engine for every tick by main
	* @param engine			The engine + game to update using
	* @param deltaTime		Time since last update (In seconds)
	*/
	void HandleUpdate(const float& deltaTime);

protected:
	/**
	* Called from handle update, at desired tickrate (Previously set)
	* @param engine			The engine + game to update using
	* @param deltaTime		Time since last update (In seconds)
	*/
	virtual void Update(const float& deltaTime) = 0;


	/**
	* Getters and setters
	*/
public:
	inline const bool& IsHost() const { return bIsHost; }
	inline const bool& IsRemote() const { return !bIsHost; }

	inline const bool& IsConnected() const { return bIsConnected; }
	inline const NetIdentity& GetSessionIdentity() const { return m_netIdentity; }

	inline const uint32& GetTickRate() const { return m_tickRate; }
	inline void SetTickRate(const uint32& v) { m_tickRate = (v == 0 ? 1 : v); m_sleepRate = 1.0f / (float)m_tickRate; }
};