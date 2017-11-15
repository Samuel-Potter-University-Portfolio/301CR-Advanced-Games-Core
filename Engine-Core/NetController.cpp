#include "Includes\Core\NetController.h"

#include "Includes\Core\NetHostSession.h"
#include "Includes\Core\NetRemoteSession.h"



NetController::NetController(const Engine* engine) :
	m_engine(engine)
{
	m_activeSession = nullptr;
}


NetController::~NetController()
{
	if (m_activeSession != nullptr)
		delete m_activeSession;

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

void NetController::HandleUpdate(const float& deltaTime) 
{
	if (m_activeSession != nullptr)
	{
		if(m_activeSession->IsConnected())
			m_activeSession->MainUpdate(deltaTime);
		else
		{
			// Close session
			delete m_activeSession;
			m_activeSession = nullptr;
		}
	}
}

bool NetController::HostSession(const NetIdentity& host)
{
	if (m_activeSession != nullptr)
	{
		LOG_ERROR("Cannot host session, as active session already exist on %s:%i", m_activeSession->GetSessionIdentity().ip.toString().c_str(), m_activeSession->GetSessionIdentity().port);
		return false;
	}

	NetHostSession* session = new NetHostSession(m_engine->GetGame(), host);
	if (!session->Start())
	{
		delete session;
		return false;
	}

	m_activeSession = session;
	return true;
}

bool NetController::JoinSession(const NetIdentity& remote)
{
	if (m_activeSession != nullptr)
	{
		LOG_ERROR("Cannot connect to session, as active session already exist on %s:%i", m_activeSession->GetSessionIdentity().ip.toString().c_str(), m_activeSession->GetSessionIdentity().port);
		return false;
	}

	NetRemoteSession* session = new NetRemoteSession(m_engine->GetGame(), remote);
	if (!session->Start())
	{
		delete session;
		return false;
	}

	m_activeSession = session;
	return true;
}