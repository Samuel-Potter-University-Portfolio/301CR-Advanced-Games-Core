#pragma once
#include "Common.h"
#include "Engine.h"
#include "EngineMemory.h"

#include "NetSocket.h"
#include "NetSocketTcp.h"
#include "NetSocketUdp.h"
#include "NetSession.h"


#include <list>


class Engine;


/**
* Vendor for sockets (Ensures all sockets leaving this controller will be cleaned up correctly)
*/
class NetController
{
private:
	std::list<NetSocket*> m_activeSockets;
	const Engine* m_engine;

	NetSession* m_activeSession;

public:
	NetController(const Engine* engine);
	~NetController();

	/**
	* Callback from engine for every tick by main
	* @param engine			The engine + game to update using
	* @param deltaTime		Time since last update (In seconds)
	*/
	void HandleUpdate(const float& deltaTime);

	/**
	* Build a new tcp socket
	* @returns The new socket (Memory managed by this controller)
	*/
	NetSocketTcp* BuildTcpSocket();

	/**
	* Build a new udp socket
	* @returns The new socket (Memory managed by this controller)
	*/
	NetSocketUdp* BuildUdpSocket();


	/**
	* Attempt to host a new game session over this network identity
	* @param host			The ip:port to host over
	* @returns If host session successfully opens
	*/
	bool HostSession(const NetIdentity& host);

	/**
	* Attempt to connect to a remotely hosted game session at this network identity
	* @param remote			The ip:port to connect to
	* @returns If successfully attempt to talk to remote session
	*/
	bool JoinSession(const NetIdentity& remote);


	/**
	* Getters and setters
	*/
public:
	inline NetSession* GetSession() const { return m_activeSession; }
};


