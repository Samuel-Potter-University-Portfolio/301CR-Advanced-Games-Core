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
		// Front is where to store temp value
		sf::TcpSocket* buffSock = m_activeConnections.front();

		// Add all active connections
		while (lsnr->accept(*buffSock) == sf::Socket::Done)
		{
			// Push active socket into chain
			m_activeConnections.push_back(buffSock);
			buffSock == new sf::TcpSocket;
			m_activeConnections[0] = buffSock;
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
			packet.sourceAddress = socket->getRemoteAddress();
			packet.sourcePort = socket->getRemotePort();

			while (socket->receive(packet.data, NET_PACKET_MAX, packet.dataCount) == sf::Socket::Done)
			{
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
		bool received = false;
		RawNetPacket packet;
		packet.sourceAddress = m_address;
		packet.sourcePort = m_port;

		while (sock->receive(packet.data, NET_PACKET_MAX, packet.dataCount) == sf::Socket::Done)
		{
			outPackets.emplace_back(packet);
			received = true;
		}
		return received;
	}
}

bool NetSocketTcp::Send(const uint8* data, uint32 count)
{
	if (m_socket == nullptr)
		return false;

	if (bIsListener)
	{
		// Update active connections
		for (int i = 1; i < m_activeConnections.size(); ++i)
		{
			sf::TcpSocket* socket = m_activeConnections[i];

			// Ignore disconnections
			if (socket->getRemoteAddress() == sf::IpAddress::None)
				continue;

			socket->send(data, count);
		}
		return true;
	}
	else
	{
		sf::TcpSocket* sock = (sf::TcpSocket*)m_socket;
		return (sock->send(data, count) == sf::Socket::Done);
	}
}

bool NetSocketTcp::Listen(uint16 port, sf::IpAddress address)
{
	if (m_socket != nullptr)
	{
		LOG_ERROR("Socket cannot listen as it's already in use");
		return false;
	}

	// Setup listener
	sf::TcpListener* lsnr = new sf::TcpListener;
	if (lsnr->listen(port, address) != sf::Socket::Done)
	{
		LOG_ERROR("Failed to setup TCP listener on port %i", port);
		return false;
	}

	m_socket = lsnr;
	m_address = address;
	m_port = port;
	bIsListener = true;

	// Add socket to reuse later
	m_activeConnections.push_back(new sf::TcpSocket);

	m_socket->setBlocking(false);
	return true;
}

bool NetSocketTcp::Connect(sf::IpAddress address, uint16 port)
{
	if (m_socket != nullptr)
	{
		LOG_ERROR("Socket cannot connect as it's already in use");
		return false;
	}

	// Connect socket
	sf::TcpSocket* sock = new sf::TcpSocket;
	if (sock->connect(address, port) != sf::Socket::Done)
	{
		LOG_ERROR("Failed to setup TCP socket to connection %s:%i", address.toString().c_str(), port);
		return false;
	}

	m_socket = sock;
	m_address = address;
	m_port = port;
	bIsListener = false;

	m_socket->setBlocking(false);
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
	return true;
}
