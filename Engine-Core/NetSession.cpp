#include "Includes\Core\NetSession.h"
#include "Includes\Core\NetController.h"

#include "Includes\Core\Engine.h"
#include "Includes\Core\Logger.h"



NetSession::NetSession(const Engine* engine, const NetIdentity identity) :
	m_engine(engine), m_netIdentity(identity)
{
}

NetSession::~NetSession()
{
}


	/*
	NetSession* session = new NetSession;
	if (!session->m_TcpSocket.Connect(remote))
	{
		LOG_ERROR("Unable to connect to net session (%s:%i). TCP error.", remote.ip.toString().c_str(), remote.port);
		delete session;
		return nullptr;
	}
	if (!session->m_UdpSocket.Connect(remote))
	{
		LOG_ERROR("Unable to connect to net session (%s:%i). UDP error.", remote.ip.toString().c_str(), remote.port);
		delete session;
		return nullptr;
	}
	
	session->bIsConnected = true;
	session->bIsHost = false;

	LOG("Remote net session correctly initialized on (%s:%i)", remote.ip.toString().c_str(), remote.port);
	return session;
	*/

void NetSession::HandleUpdate(const float& deltaTime)
{
	// Only tick at desired rate
	m_tickTimer -= deltaTime;
	if (m_tickTimer > 0)
		return;
	m_tickTimer += m_sleepRate;

	Update(deltaTime);
}
