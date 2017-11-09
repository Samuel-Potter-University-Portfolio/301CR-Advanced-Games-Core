#pragma once
#include "Common.h"
#include "NetSession.h"


class Game;



/**
* Represents a single player currently in this session
* (Relevent information will replicate between clients and host)
*/
class CORE_API NetPlayer
{
private:
	friend class NetSession;
	friend class NetHostSession;
	friend class NetRemoteSession;
	
	/** The network identity of this player (Only retrievable by host) */
	NetIdentity m_identity;

	uint16 m_uniqueId;
	bool bIsLocal = false;


public:
	NetPlayer();
	~NetPlayer();

	/**
	* Getters & Setters
	*/
public:
	/** Is this player controlled locally */
	inline const bool& IsLocal() const { return bIsLocal; }
	inline const uint32& GetUniqueID() const { return m_uniqueId; }
};

