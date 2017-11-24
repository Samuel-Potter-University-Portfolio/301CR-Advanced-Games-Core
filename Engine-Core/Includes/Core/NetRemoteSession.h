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
	OPlayerController* m_localController;
	float m_inactivityTimer = 0;
	const float m_maxInactivityTime = 15.0f;

public:
	NetRemoteSession(Game* game, const NetIdentity identity);
	virtual ~NetRemoteSession();

	/**
	* Attempt to start up the session at/on this identity
	* @returns If setup correctly
	*/
	virtual bool Start() override;

	/**
	* Callback every time there should be a network update (IO to be polled/pushed)
	* @param deltaTime		Time since last update (In seconds)
	*/
	virtual void NetUpdate(const float& deltaTime) override;

private:
	/**
	* Encode the client handshake to be sent to a server
	* @param outBuffer			Where to store the handshake
	*/
	void EncodeHandshake(ByteBuffer& outBuffer);
	/**
	* Decode the server's response to the client handshake
	* @param inbuffer			Where to read the handshake from
	* @param outPlayer			Where to store the controller, if the handshake is successful
	* @returns The response code that was sent to the client
	*/
	NetResponseCode DecodeHandshakeResponse(ByteBuffer& inBuffer, OPlayerController*& outPlayer);

	/**
	* Make sure that the client is connected to the server
	* @returns If this player is currently connected to the server
	*/
	bool EnsureConnection();


	/**
	* Getters and setters
	*/
public:
	inline const LocalClientStatus& GetConnectionStatus() const { return m_clientStatus; }
};

