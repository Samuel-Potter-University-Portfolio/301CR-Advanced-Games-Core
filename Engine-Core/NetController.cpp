#include "Includes\Core\NetController.h"



NetController::NetController()
{
}


NetController::~NetController()
{
	for (NetSocket* socket : m_activeSockets)
		delete socket;
}

NetSocketTcp* NetController::BuildTcpSocket() 
{
	NetSocketTcp* socket = new NetSocketTcp;
	m_activeSockets.push_back(socket);
	return socket;
}

NetSocketUdp* NetController::BuildUdpSocket() 
{
	NetSocketUdp* socket = new NetSocketUdp;
	m_activeSockets.push_back(socket);
	return socket;
}
