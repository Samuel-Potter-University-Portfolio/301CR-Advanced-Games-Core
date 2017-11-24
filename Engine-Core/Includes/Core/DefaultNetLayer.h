#pragma once
#include "NetLayer.h"


#define LAYER_FLAG_PASSWORD 1

/**
* Default net layer that will be used
* Provides: 
*  -Player controller naming in handshake
*  -Passworded connections
*/
class DefaultNetLayer : public NetLayer
{
	CLASS_BODY()
protected:
	string m_password;

public:
	/**
	* Callback for when this layer starts to be used
	*/
	virtual void OnBegin();

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


	/**
	* Getters & Setters
	*/
public:
	void SetPassword(const string& pass);
};

