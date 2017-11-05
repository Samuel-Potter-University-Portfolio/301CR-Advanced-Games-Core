#include "Includes\Core\NetSession.h"
#include "Includes\Core\NetController.h"
#include "Includes\Core\Logger.h"


NetSession::NetSession() : tickRate(1)
{
}

NetSession::~NetSession()
{
	LOG("NetSession closed.");
}


NetSession* NetSession::HostSession(const NetIdentity& host)
{
	NetSession* session = new NetSession;
	if (!session->m_TcpSocket.Listen(host))
	{
		LOG_ERROR("Unable to open net session on (%s:%i). TCP socket not openned (Maybe the port is already in use).", host.ip.toString().c_str(), host.port);
		delete session;
		return nullptr;
	}
	if (!session->m_UdpSocket.Listen(host))
	{
		LOG_ERROR("Unable to open net session on (%s:%i). UDP socket not openned (Maybe the port is already in use).", host.ip.toString().c_str(), host.port);
		delete session;
		return nullptr;
	}

	session->bIsConnected = true;
	session->bIsHost = true;

	LOG("Host net session correctly initialized on (%s:%i)", host.ip.toString().c_str(), host.port);
	return session;
}

NetSession* NetSession::JoinSession(const NetIdentity& remote) 
{
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
}

void NetSession::Update(Engine* engine, const float& deltaTime)
{
	// Only tick at desired rate
	m_tickTimer -= deltaTime;
	if (m_tickTimer > 0)
		return;
	m_tickTimer += desiredTickRate;


	// Read from remote
	{
		std::vector<RawNetPacket> packets;
		if (m_UdpSocket.Poll(packets))
		{
			// TODO - Do
			for (RawNetPacket& p : packets)
				LOG("UDP %i from %s:%i", p.dataCount, p.source.ip.toString().c_str(), p.source.port)
		}
		
		packets.clear();
		if (m_TcpSocket.Poll(packets))
		{
			// TODO - Do
			for (RawNetPacket& p : packets)
				LOG("TCP %i from %s:%i", p.dataCount, p.source.ip.toString().c_str(), p.source.port)
		}
	}
}
