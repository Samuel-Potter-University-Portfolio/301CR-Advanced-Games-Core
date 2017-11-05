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
* Holds basic information about a connection
*/
struct CORE_API NetIdentity
{
	sf::IpAddress	ip;
	uint16			port;

	NetIdentity(sf::IpAddress ip = sf::IpAddress::LocalHost, uint16 port = 0) : ip(ip), port(port) 
	{}

	inline bool operator<(const NetIdentity& other) const
	{
		return ip < other.ip || (ip == other.ip && port < other.port);
	}

	inline bool operator==(const NetIdentity& other) const
	{
		return ip == other.ip && port == other.port;
	}
	inline bool operator!=(const NetIdentity& other) const
	{
		return ip != other.ip || port != other.port;
	}
};


/**
* Holds the raw packet data for when it is read by the user
*/
struct CORE_API RawNetPacket
{
	NetIdentity		source;
	uint8			data[NET_PACKET_MAX];
	uint32			dataCount;
};


/**
* Represents a socketed connection for either listening or requesting data
*/
class CORE_API NetSocket
{
private:
	SocketType m_socketType;

protected:
	NetIdentity m_identity;

	bool bIsReusable = false;
	bool bIsOpen = false;
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
	bool Send(const uint8* data, uint32 count) { return SendTo(data, count, m_identity); }

	/**
	* Attempt to send data through this socket
	* @param data		Pointer to data to send
	* @param count		The size of data to send
	* @param address	Destination address
	* @param port		Destination port
	* @returns Has the data been successfully sent
	*/
	virtual bool SendTo(const uint8* data, uint32 count, NetIdentity identity) = 0;

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
	virtual bool Listen(NetIdentity identity) = 0;

	/**
	* Open a connect to the given destination
	*/
	virtual bool Connect(NetIdentity identity) = 0;

	/**
	* Getters and setters
	*/
public:
	inline const SocketType& GetSocketType() const { return m_socketType; }
	inline const NetIdentity& GetNetIdentity() const { return m_identity; }

	inline void MarkAsReusable() { bIsReusable = true; }
	inline const bool& IsReusable() const { return bIsReusable; }

	inline const bool& IsOpen() const { return bIsOpen; }
	inline const bool& IsListener() const { return bIsListener; }
};