#pragma once
#include "Common.h"

#include "NetSocketTcp.h"
#include "NetSocketUdp.h"

#include "Object.h"
#include "Actor.h"
#include "PlayerController.h"

#include "NetLayer.h"


class Game;



/**
* What is this current message trying to do
*/
enum class NetMessage : uint8 
{
	Nothing					= 0,
	/// Net serialized base encoded message
	NetObjectMessage		= 1,
};

/**
* Used to identify what a NetObjectMessage is refering to
*/
enum class NetObjectReferenceType : uint8
{
	Object		= 0,
	Actor		= 1
};
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

	std::vector<OPlayerController*> m_playerControllers;
	uint16 m_maxPlayerCount = 10;

	std::vector<NetObjectDeletion> m_deletionQueue;

public:
	NetSession(Game* game, const NetIdentity identity);
	virtual ~NetSession();

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
	* @param target				The player who is the target for this data (or nullptr, if intended for the host)
	* @param object				The object to encode
	* @param buffer				Where to store all information
	* @param socketType			The socket type this content will be sent over
	*/
	void EncodeNetObject(const OPlayerController* target, OObject* object, ByteBuffer& buffer, const SocketType& socketType);
	/**
	* Decode information received about an object
	* @param source				The player who is the source of this data (or nullptr, if from the server)
	* @param isActor			Is this message refering to an actor (If false it refers to an object)
	* @param buffer				Where to store all information
	* @param socketType			The socket type this content will be sent over
	* @param justCleanUp		The buffer will be read, however no actors/objects will be updated (Used to skip messages)
	*/
	void DecodeNetObject(const OPlayerController* source, const bool& isActor, ByteBuffer& buffer, const SocketType& socketType, const bool& justCleanUp);

	/**
	* Encode any relevant information to be sent out this net update
	* @param target				The player who is the target for this data (or nullptr, if intended for the host)
	* @param buffer				Where to store all information
	* @param socketType			The socket type this content will be sent over
	*/
	void NetEncode(const OPlayerController* target, ByteBuffer& buffer, const SocketType& socketType);
	/**
	* Decode any information that was received this net update
	* @param source				The player who is the source of this data (or nullptr, if from the server)
	* @param buffer				Where to read all the information
	* @param socketType			The socket type this content was read over
	*/
	void NetDecode(const OPlayerController* source, ByteBuffer& buffer, const SocketType& socketType);


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

	inline void SetMaxPlayerCount(const uint16& count) { m_maxPlayerCount = count; }
	inline const uint16& GetMaxPlayerCount() const { return m_maxPlayerCount; }

	inline const NetIdentity& GetSessionIdentity() const { return m_netIdentity; }
	inline const uint16& GetSessionNetID() const { return m_sessionNetId; }

	inline const uint32& GetTickRate() const { return m_tickRate; }
	inline void SetTickRate(const uint32& v) { m_tickRate = (v == 0 ? 1 : v); m_sleepRate = 1.0f / (float)m_tickRate; }
};