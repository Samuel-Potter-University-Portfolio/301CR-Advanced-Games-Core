#include "Includes\Core\NetSocketTcp.h"



NetSocketTcp::NetSocketTcp() : NetSocket(TCP)
{
}

NetSocketTcp::~NetSocketTcp() 
{
	if (m_socket != nullptr)
		Close();
}

bool NetSocketTcp::Poll(std::vector<RawNetPacket>& outPackets)
{
	if (m_socket == nullptr)
		return false;
	

	if (bIsListener)
	{
		sf::TcpListener* lsnr = (sf::TcpListener*)m_socket;
		
		// Accept new connections
		{
			// Front is where to store temp value
			sf::TcpSocket* buffSock = m_activeConnections.front();

			// Add all active connections
			while (lsnr->accept(*buffSock) == sf::Socket::Done)
			{
				// Push active socket into chain
				m_activeConnections.push_back(buffSock);
				buffSock = new sf::TcpSocket;
				m_activeConnections[0] = buffSock;
			}
		}


		bool received = false;

		// Update active connections
		for (int i = 1; i < m_activeConnections.size(); ++i)
		{
			sf::TcpSocket* socket = m_activeConnections[i];

			// Handle disconnections
			if (socket->getRemoteAddress() == sf::IpAddress::None)
			{
				m_activeConnections.erase(m_activeConnections.begin() + i);
				--i;
				continue;
			}


			// Attempt to read all data
			RawNetPacket packet;
			packet.source.ip = socket->getRemoteAddress();
			packet.source.port = socket->getRemotePort();

			
			uint8 data[NET_PACKET_MAX];
			uint32 dataCount;

			if (socket->receive(data, NET_PACKET_MAX, dataCount) == sf::Socket::Done)
			{
				// Put all data into a single packet, for ease of use
				packet.buffer.Push(data, dataCount);
				outPackets.emplace_back(packet);
				received = true;
			}
		}

		return received;
	}
	else 
	{
		sf::TcpSocket* sock = (sf::TcpSocket*)m_socket;

		// Attempt to read all data
		RawNetPacket packet;
		packet.source = m_identity;

		uint8 data[NET_PACKET_MAX];
		uint32 dataCount;

		if (sock->receive(data, NET_PACKET_MAX, dataCount) == sf::Socket::Done)
		{
			// Put all data into a single packet, for ease of use
			packet.buffer.Push(data, dataCount);
			outPackets.emplace_back(packet);
			return true;
		}

		return false;
	}
}

bool NetSocketTcp::SendTo(const uint8* data, uint32 count, NetIdentity identity)
{
	if (m_socket == nullptr)
		return false;

	if (bIsListener)
	{
		// Update active connections
		for (int i = 1; i < m_activeConnections.size(); ++i)
		{
			sf::TcpSocket* socket = m_activeConnections[i];

			if (socket->getRemoteAddress() == identity.ip && socket->getRemotePort() == identity.port)
				return socket->send(data, count) == sf::Socket::Status::Done;
		}
		return false;
	}
	else
	{
		sf::TcpSocket* socket = (sf::TcpSocket*)m_socket;
		if (socket->getRemoteAddress() == identity.ip && socket->getRemotePort() == identity.port)
			return (socket->send(data, count) == sf::Socket::Done);
		else
			return false;
	}
}

bool NetSocketTcp::Listen(NetIdentity identity)
{
	if (m_socket != nullptr)
	{
		LOG_ERROR("Socket cannot listen as it's already in use");
		return false;
	}

	// Setup listener
	sf::TcpListener* lsnr = new sf::TcpListener;
	if (lsnr->listen(identity.port, identity.ip) != sf::Socket::Done)
	{
		LOG_ERROR("Failed to setup TCP listener on %s:%i", identity.ip.toString().c_str(), identity.port);
		return false;
	}

	m_socket = lsnr;
	m_identity = identity;
	bIsListener = true;

	// Add socket to reuse later
	m_activeConnections.push_back(new sf::TcpSocket);

	m_socket->setBlocking(false);
	bIsOpen = true;
	return true;
}

bool NetSocketTcp::Connect(NetIdentity identity)
{
	if (m_socket != nullptr)
	{
		LOG_ERROR("Socket cannot connect as it's already in use");
		return false;
	}

	// Connect socket
	sf::TcpSocket* sock = new sf::TcpSocket;
	if (sock->connect(identity.ip, identity.port) != sf::Socket::Done)
	{
		LOG_ERROR("Failed to setup TCP socket to connection %s:%i", identity.ip.toString().c_str(), identity.port);
		return false;
	}

	m_socket = sock;
	m_identity = identity;
	bIsListener = false;

	m_socket->setBlocking(false);
	bIsOpen = true;
	return true;
}

bool NetSocketTcp::Close()
{
	if (m_socket == nullptr)
	{
		LOG_ERROR("Socket cannot close as it's not in use");
		return false;
	}

	if (bIsListener)
	{
		sf::TcpListener* lsnr = (sf::TcpListener*)m_socket;
		lsnr->close();

		// Cleanup connections
		for (sf::TcpSocket* socket : m_activeConnections)
			delete socket;
		m_activeConnections.clear();
		delete lsnr;
	}
	else
	{
		sf::TcpSocket* sock = (sf::TcpSocket*)m_socket;
		sock->disconnect();
		delete sock;
	}
	m_socket = nullptr;
	bIsOpen = false;
	return true;
}
