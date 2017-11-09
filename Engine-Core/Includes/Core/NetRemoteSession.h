#pragma once
#include "NetSession.h"


enum LocalClientStatus 
{
	PreHandshake,
	WaitingOnHandshake,
	Connected,
	Disconnected
};

/**
* Represents a connection to a remotely hosted server
*/
class CORE_API NetRemoteSession : public NetSession
{
private:
	LocalClientStatus m_clientStatus;
	NetResponseCode m_connectionStatus;

public:
	NetRemoteSession(const Engine* engine, const NetIdentity identity);
	virtual ~NetRemoteSession();

	/**
	* Attempt to start up the session at/on this identity
	* @returns If setup correctly
	*/
	virtual bool Start();

	/**
	* Called from handle update, at desired tickrate (Previously set)
	* @param engine			The engine + game to update using
	* @param deltaTime		Time since last update (In seconds)
	*/
	virtual void Update(const float& deltaTime);

private:
	/**
	* Attempt to validate the response from the handshake
	* @param packet			The response from the server
	* @return If the handshake has been resolved (Not whether it has been accepted)
	*/
	bool ValidateHandshakeResponse(RawNetPacket& packet);

	/**
	* Make sure that the client is connected to the server
	* @returns If this player is currently connected to the server
	*/
	bool EnsureConnection();


protected:
	/**
	* Encode any relevant information to be sent out this net update
	* @param buffer			Where to store all information
	* @param socketType		The socket type this content will be sent over
	*/
	virtual void NetEncode(ByteBuffer& buffer, const SocketType& socketType);


	/**
	* Getters and setters
	*/
public:
	inline const NetResponseCode& GetConnectionStatus() const { return m_connectionStatus; }
};

