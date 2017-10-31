#include "Includes\Core\NetSocketUdp.h"



NetSocketUdp::NetSocketUdp() : NetSocket(UDP)
{
}

NetSocketUdp::~NetSocketUdp()
{
	if (m_socket != nullptr)
		Close();
}

bool NetSocketUdp::Poll(std::vector<RawNetPacket>& outPackets)
{
	if (m_socket == nullptr)
		return false;

	sf::UdpSocket* sock = (sf::UdpSocket*)m_socket;

	// Attempt to read all data
	bool recieved = false;
	RawNetPacket packet;
	while (sock->receive(packet.data, NET_PACKET_MAX, packet.dataCount, packet.sourceAddress, packet.sourcePort) == sf::Socket::Done)
	{
		outPackets.emplace_back(packet);
		recieved = true;
	}
	return recieved;
}

bool NetSocketUdp::Send(const uint8* data, uint32 count)
{
	if (m_socket == nullptr)
		return false;

	sf::UdpSocket* sock = (sf::UdpSocket*)m_socket;
	return (sock->send(data, count, m_address, m_port) == sf::Socket::Done);
}

bool NetSocketUdp::Listen(uint16 port, sf::IpAddress address)
{
	if (m_socket != nullptr)
	{
		LOG_ERROR("Socket cannot listen as it's already in use")
			return false;
	}

	// Open socket on port
	sf::UdpSocket* sock = new sf::UdpSocket;
	if (sock->bind(port, address) != sf::Socket::Done)
	{
		LOG_ERROR("Failed to setup UDP listener on port %i", port);
		return false;
	}

	m_socket = sock;
	m_address = address;
	m_port = port;
	bIsListener = true;

	m_socket->setBlocking(false);
	return true;
}

bool NetSocketUdp::Connect(sf::IpAddress address, uint16 port)
{
	if (m_socket != nullptr)
	{
		LOG_ERROR("Socket cannot connect as it's already in use")
			return false;
	}

	// Open socket on any free port
	sf::UdpSocket* sock = new sf::UdpSocket;
	if (sock->bind(sf::Socket::AnyPort) != sf::Socket::Done)
	{
		LOG_ERROR("Failed to setup UDP socket to connection %s:%i", address.toString().c_str(), port);
		return false;
	}

	m_socket = sock;
	m_address = address;
	m_port = port;
	bIsListener = false;

	m_socket->setBlocking(false);
	return true;
}

bool NetSocketUdp::Close()
{
	if (m_socket == nullptr)
	{
		LOG_ERROR("Socket cannot close as it's not in use")
			return false;
	}
	
	// Release port
	sf::UdpSocket* sock = (sf::UdpSocket*)m_socket;
	sock->unbind();
	delete sock;
	m_socket = nullptr;
	return true;
}