#pragma once
#include "Object.h"


/**
* Represents a player that is currently connected to the server
*/
class CORE_API OPlayerController : public OObject
{
	CLASS_BODY()
private:
	string m_playerName;

public:
	OPlayerController();

	virtual void OnBegin() override;
	virtual void OnDestroy() override;
	
	/**
	* RPC overrides
	*/
protected:
	virtual bool RegisterRPCs(const char* func, RPCInfo& outInfo) const override;
	virtual bool ExecuteRPC(uint16& id, ByteBuffer& params) override;


	/**
	* Getters & Setters
	*/
public:
	/**
	* Sets this player's name
	* @param name		The name to change it to
	*/
	void SetPlayerName(string name);
	inline const string GetPlayerName() const { return m_playerName; }

	/** Is this controller owned by this client */
	inline bool IsLocal() const { return IsNetOwner(); }
};

