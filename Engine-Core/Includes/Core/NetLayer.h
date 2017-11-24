#pragma once
#include "Common.h"
#include "ManagedClass.h"
#include "ByteBuffer.h"

#include "NetSocket.h"



/**
* What sort of requests can be performed during a handshake
*/
enum class NetRequestType : uint16
{
	Ping = 0,
	Connect = 200,
	Query = 201
};

/**
* Response codes used in the initial handshake
*/
enum class NetResponseCode : uint16
{
	Unknown = 0,
	WaitingOnUpStream = 1,	// Layer is waiting for upstream response until it's ready to respond with the actual response

	Accepted = 200,
	Responded = 201,

	BadRequest = 400,
	Banned = 401,
	BadAuthentication = 403,
	BadVersions = 426,

	ServerInternalError = 500,
	ServerFull = 512
};


class Game;
class NetSession;
class OPlayerController;


/**
* Net layer sits on top of the NetSession allowing for additional behaviour to be added
*/
class CORE_API NetLayer : public ManagedObject
{
	CLASS_BODY()
private:
	Game* m_game;
	NetSession* m_session;

protected:
	/// Bit flags describing this connection (Decided by NetLayer children)
	uint32 m_connFlags = 0;
public:
	/**
	* Initializes this layer
	* @param game			The game currently in use
	* @param session		The session this layer is being used on
	*/
	void Initialize(Game* game, NetSession* session);
	virtual ~NetLayer();


	/**
	* Callback for when this layer starts to be used
	*/
	virtual void OnBegin() {}
	/**
	* Callback for every network tick 
	* @param deltaTime			Time since last tick (In seconds)
	*/
	virtual void OnNetTick(const float& deltaTime) {}


public:
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
	inline Game* GetGame() const { return m_game; }
	inline NetSession* GetSession() const { return m_session; }

	inline uint32 GetConnectionBitFlags() const { return m_connFlags; }
};

