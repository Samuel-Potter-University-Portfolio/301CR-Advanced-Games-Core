#pragma once
#include "NetSocket.h"



/**
* Represents a single client currently connected to the server
*  - Held and managed by NetSession
*/
class CORE_API NetClient
{
private:
	NetIdentity m_identity;

public:
	NetClient(NetIdentity identity);
	~NetClient();

	/**
	* Getters and setters
	*/
public:
	inline const NetIdentity& GetNetIdentity() const { return m_identity; }
};

