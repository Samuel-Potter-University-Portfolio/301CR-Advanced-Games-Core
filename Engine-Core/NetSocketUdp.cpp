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


	// Attempt to read all data
	bool recieved = false;

	NetIdentity source;
	uint8 data[NET_PACKET_MAX];
	uint32 dataCount;

	while (m_socket->receive(data, NET_PACKET_MAX, dataCount, source.ip, source.port) == sf::Socket::Done)
	{
		// Only listen to server's traffic, if not listener
		if (!bIsListener && source != m_identity)
			continue;

		// Format packet
		RawNetPacket packet;
		packet.source = source;
		packet.buffer.Push(data, dataCount);

		outPackets.emplace_back(packet);
		recieved = true;
	}
	return recieved;
}

bool NetSocketUdp::SendTo(const uint8* data, uint32 count, NetIdentity identity)
{
	if (m_socket == nullptr)
		return false;

	if (count >= NET_PACKET_MAX)
	{
		LOG_ERROR("Packet exceeds maximum supported size (%i)", NET_PACKET_MAX);
		return false;
	}

	return (m_socket->send(data, count, identity.ip, identity.port) == sf::Socket::Done);
}

bool NetSocketUdp::Listen(NetIdentity identity)
{
	if (m_socket != nullptr)
	{
		LOG_ERROR("Socket cannot listen as it's already in use");
		return false;
	}

	// Open socket on port
	m_socket = new sf::UdpSocket;
	if (m_socket->bind(identity.port, identity.ip) != sf::Socket::Done)
	{
		LOG_ERROR("Failed to setup UDP listener on %s:%i", identity.ip.toString().c_str(), identity.port);
		delete m_socket;
		m_socket = nullptr;
		return false;
	}

	m_identity = identity;
	bIsListener = true;

	m_socket->setBlocking(false);
	bIsOpen = true;
	return true;
}

bool NetSocketUdp::Connect(NetIdentity identity)
{
	if (m_socket != nullptr)
	{
		LOG_ERROR("Socket cannot connect as it's already in use");
		return false;
	}

	// Open socket on any free port
	m_socket = new sf::UdpSocket;
	if (m_socket->bind(sf::Socket::AnyPort) != sf::Socket::Done)
	{
		LOG_ERROR("Failed to setup UDP socket to connection %s:%i", identity.ip.toString().c_str(), identity.port);
		delete m_socket;
		m_socket = nullptr;
		return false;
	}

	m_identity = identity;
	bIsListener = false;

	m_socket->setBlocking(false);
	bIsOpen = true;
	return true;
}

bool NetSocketUdp::Close()
{
	if (m_socket == nullptr)
	{
		LOG_ERROR("Socket cannot close as it's not in use");
		return false;
	}
	
	// Release port
	m_socket->unbind();
	delete m_socket;
	m_socket = nullptr;
	bIsOpen = false;
	return true;
}