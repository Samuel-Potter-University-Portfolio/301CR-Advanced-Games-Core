#pragma once
#include "Common.h"

#include "NetSocketTcp.h"
#include "NetSocketUdp.h"

#include "Object.h"
#include "Actor.h"
#include "PlayerController.h"

#include "NetLayer.h"


class Game;
struct NetPlayerConnection;


/**
* Describes what a NetObjectMessage is describing
*/
enum class NetObjectMethod : uint8
{
	Empty		= 0,
	New			= 1,
	Delete		= 2,
	Update		= 3,
};


/** Holds information about an object that has been destroyed */
struct NetObjectDeletion 
{
	bool		bIsActor;
	uint16		netId;
};



/**
* Represents the connection between a player and a server or a server and players
*/
class CORE_API NetSession 
{
private:
	const NetIdentity m_netIdentity;
	Game* m_game;

	uint16 m_playerIdCounter;
	uint16 m_objectNetIdCounter;
	uint16 m_actorNetIdCounter;

	uint32 m_tickRate = 30;
	float m_sleepRate = 1.0f / (float)m_tickRate;
	float m_tickTimer = 0.0f;

protected:
	NetSocketTcp m_TcpSocket;
	NetSocketUdp m_UdpSocket;

	NetLayer* m_netLayer;
	uint16 m_sessionNetId;
	bool bIsHost = false;
	bool bIsConnected = false;
	
	string m_sessionName;

	uint16 m_maxPlayerCount = 10;
	std::vector<NetObjectDeletion> m_deletionQueue;

public:
	NetSession(Game* game, const NetIdentity identity);
	virtual ~NetSession();

	/**
	* Setup the NetLayer to use for this session
	* @param layerType			The layer class to use
	* @param configLayer		Callback to use to configure this layer
	*/
	void SetupLayer(SubClassOf<NetLayer> layerType, ConfigLayer configLayer);

	/**
	* Attempt to start up the session at/on this identity
	* @returns If setup correctly
	*/
	virtual bool Start() { return false; }

	/**
	* Callback from engine for every tick by main loop
	* @param deltaTime		Time since last update (In seconds)
	*/
	void MainUpdate(const float& deltaTime);

	/**
	* Callback for when an object gets destroyed
	* @param object			Pointer to object (Only safe for duration of function call)
	*/
	void OnNetObjectDestroy(const OObject* object);

protected:
	/**
	* Callback for before a net update occurs
	*/
	void PreNetUpdate();
	/**
	* Callback every time there should be a network update (IO to be polled/pushed)
	* @param deltaTime		Time since last update (In seconds)
	*/
	virtual void NetUpdate(const float& deltaTime) {}
	/**
	* Callback for after a net update occurs
	*/
	void PostNetUpdate();


protected:
	/**
	* Encode any information to be sent out, for this object
	* @param target				The client who is the target for this data (or nullptr, if intended for the host)
	* @param object				The object to encode
	* @param buffer				Where to store all information
	* @param socketType			The socket type this content will be sent over
	* @param encodeAsNew		Encode this object as if it's new to the client
	*/
	void EncodeNetObject(NetPlayerConnection* target, OObject* object, ByteBuffer& buffer, const SocketType& socketType, const bool& encodeAsNew);
	/**
	* Decode information received about an object
	* @param source				The player who is the source of this data (or nullptr, if from the host)
	* @param isActor			Is this message refering to an actor (If false it refers to an object)
	* @param buffer				Where to read all the data from
	* @param socketType			The socket type this content was recieved on
	*/
	void DecodeNetObject(NetPlayerConnection* source, const bool& isActor, ByteBuffer& buffer, const SocketType& socketType);

	/**
	* Encode any relevant information to be sent out this net update
	* @param target				The client who is the target for this data (or nullptr if intended for the host)
	* @param buffer				Where to store all information
	* @param socketType			The socket type this content will be sent over
	*/
	void EncodeNetUpdate(NetPlayerConnection* target, ByteBuffer& buffer, const SocketType& socketType);
	/**
	* Decode any information that was received this net update
	* @param source					The client who is the source of this data (or nullptr, if from the host)
	* @param buffer					Where to read all the information
	* @param socketType				The socket type this content was read over
	*/
	void DecodeNetUpdate(NetPlayerConnection* source, ByteBuffer& buffer, const SocketType& socketType);


	/**
	* Getters & Setters
	*/
protected:
	inline uint16 NewPlayerID() { return m_playerIdCounter++; }
	inline uint16 NewObjectID() { return m_objectNetIdCounter++; }
	inline uint16 NewActorID() { return m_actorNetIdCounter++; }

public:
	inline Game* GetGame() const { return m_game; }
	
	inline const bool& IsHost() const { return bIsHost; }
	inline const bool& IsRemote() const { return !bIsHost; }
	inline const bool& IsConnected() const { return bIsConnected; }

	inline void SetSessionName(const string& name) { m_sessionName = name; }
	inline const string& GetSessionName() const { return m_sessionName; }

	inline void SetMaxPlayerCount(const uint16& count) { m_maxPlayerCount = count; }
	inline const uint16& GetMaxPlayerCount() const { return m_maxPlayerCount; }

	inline const NetIdentity& GetSessionIdentity() const { return m_netIdentity; }
	inline const uint16& GetSessionNetID() const { return m_sessionNetId; }

	inline const uint32& GetTickRate() const { return m_tickRate; }
	inline void SetTickRate(const uint32& v) { m_tickRate = (v == 0 ? 1 : v); m_sleepRate = 1.0f / (float)m_tickRate; }
};