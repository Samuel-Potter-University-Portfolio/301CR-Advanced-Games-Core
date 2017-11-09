#pragma once
#include "Common.h"
#include <set>

#include "NetSocketTcp.h"
#include "NetSocketUdp.h"


class Engine;
class Entity;


/**
* What sort of requests can be performed during a handshake
*/
enum class NetRequestType : uint16
{
	Ping		= 0,
	Connect		= 200,
	Query		= 201
};

/**
* Response codes used in the initial handshake
*/
enum class NetResponseCode : uint16
{
	Unknown			= 0,

	Accepted		= 200,
	Responded		= 201,

	BadRequest		= 400,
	Banned			= 401,
	BadPassword		= 403,
	BadVersions		= 426,

	ServerFull		= 512
};



/**
* What is this current message trying to do
*/
enum class NetMessage : uint8 
{
	Nothing				= 0,
	EntityMessage		= 1,
};

/**
* What this current entity message doing
*/
enum class NetEntityMethod : uint8
{
	Nothing				= 0,
	Spawn				= 1,
	Despawn				= 2,
	ControlOverride		= 3,
	Update				= 4,
};



/**
* Abstract class represents the current connection between player(s) and a server
*/
class CORE_API NetSession
{
private:
	const NetIdentity m_netIdentity;
	uint16 m_entityIdCounter; 

	std::set<uint16> m_newPlayers; // Players who have just been accepted
	uint16 m_playerIdCounter;

	uint32 m_tickRate = 20;
	float m_sleepRate = 1.0f / (float)m_tickRate;
	float m_tickTimer = 0.0f;

protected:
	NetSocketTcp m_TcpSocket;
	NetSocketUdp m_UdpSocket;
	const Engine* m_engine;

	/// Used to tell host session apart from remote
	uint16 m_netId;
	bool bIsHost = false;
	bool bIsConnected = false;

public:
	NetSession(const Engine* engine, const NetIdentity identity);
	virtual ~NetSession();

	/**
	* Attempt to start up the session at/on this identity
	* @returns If setup correctly
	*/
	virtual bool Start() = 0;

	/**
	* Callback from engine for every tick by main
	* @param engine			The engine + game to update using
	* @param deltaTime		Time since last update (In seconds)
	*/
	void HandleUpdate(const float& deltaTime);

protected:
	/**
	* Called from handle update, at desired tickrate (Previously set)
	* @param engine			The engine + game to update using
	* @param deltaTime		Time since last update (In seconds)
	*/
	virtual void Update(const float& deltaTime) = 0;


	/**
	* Encode a handshake to be sent from a client to the server
	* @param buffer			Where to store all information
	*/
	void EncodeClientHandshake(ByteBuffer& buffer);
	/**
	* Decode a handshake sent from a client to the server
	* @param inbuffer			Where read data from
	* @param outBuffer			Where write data to (To be sent back to the client as a response)
	* @param outNetId			The net id of the player (If accepted)
	* @returns The result of the handshake
	*/
	NetResponseCode DecodeClientHandshake(ByteBuffer& inbuffer, ByteBuffer& outBuffer, uint16& outNetId);
	/**
	* Decode a handshake response received from the server
	* @param buffer			Where read data from
	* @param outNetId			The net id of the player (If accepted)
	* @returns The result of the handshake
	*/
	NetResponseCode DecodeServerHandshake(ByteBuffer& buffer, uint16& outNetId);


	/**
	* Encode all relevent information about this given entity
	* @param netId			The net id of where this data will be sent to
	* @param buffer				Where to store all information
	* @param socketType			The socket type this content will be sent over
	* @param entity				The entity that we wish to encode
	*/
	void EncodeEntityMessage(const uint16& netId, ByteBuffer& buffer, const SocketType& socketType, Entity* entity);
	/**
	* Decode information about this entity message
	* @param netId			The net id of where this data has arrived from
	* @param buffer			Where read data from
	* @param socketType		The socket type this content will be sent over
	*/
	void DecodeEntityMessage(const uint16& netId, ByteBuffer& buffer, const SocketType& socketType);


	/**
	* Encode any relevant information to be sent out this net update
	* @param netId			The net id of where this data will be sent to
	* @param buffer			Where to store all information
	* @param socketType		The socket type this content will be sent over
	*/
	virtual void NetEncode(const uint16& netId, ByteBuffer& buffer, const SocketType& socketType) = 0;
	/**
	* Decode any information that was received this net update
	* @param netId			The net id of where this data has arrived from
	* @param buffer			Where to read all the information
	* @param socketType		The socket type this content was read over
	*/
	virtual void NetDecode(const uint16& netId, ByteBuffer& buffer, const SocketType& socketType);


	/**
	* Getters and setters
	*/
public:
	inline const bool& IsHost() const { return bIsHost; }
	inline const bool& IsRemote() const { return !bIsHost; }

	inline const bool& IsConnected() const { return bIsConnected; }
	inline const NetIdentity& GetSessionIdentity() const { return m_netIdentity; }
	inline const uint16& GetNetworkID() const { return m_netId; }

	inline const uint32& GetTickRate() const { return m_tickRate; }
	inline void SetTickRate(const uint32& v) { m_tickRate = (v == 0 ? 1 : v); m_sleepRate = 1.0f / (float)m_tickRate; }
};