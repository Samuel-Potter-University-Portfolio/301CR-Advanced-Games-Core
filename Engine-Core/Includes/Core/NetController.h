#pragma once
#include "Common.h"
#include "EngineMemory.h"

#include "NetSocket.h"
#include "NetSocketTcp.h"
#include "NetSocketUdp.h"


#include <list>


/**
* Vendor for sockets and networking
*/
class NetController
{
private:
	std::list<NetSocket*> m_activeSockets;

public:
	NetController();
	~NetController();

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
};

