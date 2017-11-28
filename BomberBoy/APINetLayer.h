#pragma once
#include "Core\Core-Common.h"
#include "APIController.h"


/**
* Represents layer a layer that will only accept and validate clients who have a valid login
*/
class APINetLayer : public DefaultNetLayer
{
	CLASS_BODY()
private:
	enum UserCheckState : uint8
	{
		Pending,
		Confirmed,
		Denied
	};

	OAPIController* m_controller;
	std::map<NetIdentity, UserCheckState> m_loginChecks;

public:
	APINetLayer();
	virtual ~APINetLayer();

	virtual void OnBegin() override;

	/**
	* Callback after outgoing (client to server) handshake request has been created
	* Encode any additional information here
	* @param host				The host this will go to
	* @param outBuffer			The buffer to store the handshake
	*/
	virtual void OnEncodeHandshake(const NetIdentity& host, ByteBuffer& outBuffer);

	/**
	* Callback when host receives handshake from a client where client is trying to connect to server (NetRequestType::Connect)
	* Decode any additional information here fetched here
	* -Note: outPlayer is not safe to use outside of this call
	* @param connection			The connection that is currently trying to connect
	* @param inBuffer			The buffer to read the handshake from
	* @param outPlayer			Where to create/store the player controller for this connection (If it's accepted)
	* @returns Response code to send to this source
	*/
	virtual NetResponseCode OnDecodeHandshake(const NetIdentity& connection, ByteBuffer& inBuffer, OPlayerController*& outPlayer);
};

