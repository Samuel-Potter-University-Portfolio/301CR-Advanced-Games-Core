#pragma once
#include <SFML\Network.hpp>
#include <vector>
#include "Common.h"


#define NET_PACKET_MAX 4096



enum SocketType
{
	TCP,
	UDP
};


/**
* Holds the raw packet data for when it is read by the user
*/
struct RawNetPacket
{
	sf::IpAddress	sourceAddress;
	uint16			sourcePort;
	uint8			data[NET_PACKET_MAX];
	uint32			dataCount;
};


/**
* Represents a TCP or UDP socket
*/
class CORE_API NetSocket
{
private:
	SocketType m_socketType;

protected:
	sf::Socket* m_socket;

	sf::IpAddress m_address;
	uint16 m_port;

	bool bIsListener = false;

public:
	NetSocket(SocketType type);
	virtual ~NetSocket();

	/**
	* Polls the socket attempting to retrieve and data
	* @param outPackets			Where to store all of the resulting packets
	* @returns If the poll is successful and there is packets to process
	*/
	virtual bool Poll(std::vector<RawNetPacket>& outPackets) = 0;

	/**
	* Attempt to send data through this socket
	* @param data		Pointer to data to send
	* @param count		The size of data to send
	* @returns Has the data been successfully sent
	*/
	virtual bool Send(const uint8* data, uint32 count) = 0;

	/**
	* Try to close this socket
	*/
	virtual bool Close() = 0;

	/**
	* Open this socket as a listener
	* @param port		The desired port to open on
	* @param address	The address to bind the socket onto
	* @returns Whether this successfully opens or not
	*/
	virtual bool Listen(uint16 port, sf::IpAddress address = sf::IpAddress::LocalHost) = 0;

	/**
	* Open a connect to the given destination
	*/
	virtual bool Connect(sf::IpAddress address, uint16 port) = 0;

	/**
	* Getters and setters
	*/
public:
	inline const SocketType& GetSocketType() const { return m_socketType; }

	inline const sf::IpAddress& GetAddress() const { return m_address; }
	inline const uint16& GetPort() const { return m_port; }

	inline const bool& IsOpen() const { return m_socket != nullptr; }
	inline const bool& IsListener() const { return bIsListener; }
};