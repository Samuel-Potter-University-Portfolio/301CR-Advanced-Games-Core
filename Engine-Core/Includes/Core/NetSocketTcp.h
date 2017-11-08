#pragma once
#include "NetSocket.h"



class CORE_API NetSocketTcp : public NetSocket
{
private:
	// Active connections used by this listener
	std::vector<sf::TcpSocket*> m_activeConnections;
	sf::Socket* m_socket;

public:
	NetSocketTcp(); 
	virtual ~NetSocketTcp();

	/**
	* Polls the socket attempting to retrieve and data
	* @param outPackets			Where to store all of the resulting packets
	* @returns If the poll is successful and there is packets to process
	*/
	virtual bool Poll(std::vector<RawNetPacket>& outPackets);

	/**
	* Attempt to send data through this socket
	* @param data		Pointer to data to send
	* @param count		The size of data to send
	* @param address	Destination address
	* @param port		Destination port
	* @returns Has the data been successfully sent
	*/
	virtual bool SendTo(const uint8* data, uint32 count, NetIdentity identity);

	/**
	* Try to close this socket
	*/
	virtual bool Close();

	/**
	* Open this socket as a listener
	* @param port		The desired port to open on
	* @param address	The address to bind the socket onto
	* @returns Whether this successfully opens or not
	*/
	virtual bool Listen(NetIdentity identity);

	/**
	* Open a connect to the given destination
	*/
	virtual bool Connect(NetIdentity identity);

	/**
	* Return the local identity that this socket is using (Same as identity, if listener)
	*/
	NetIdentity GetLocalIdentity();
};

