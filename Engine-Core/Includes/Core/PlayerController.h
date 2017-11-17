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
	* Callback for when a player changes their name
	*/
	virtual void OnNameChange();


	/**
	* RPC and Net var overrides
	*/
protected:
	virtual bool RegisterRPCs(const char* func, RPCInfo& outInfo) const override;
	virtual bool ExecuteRPC(uint16& id, ByteBuffer& params) override;

	virtual void RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode) override;
	virtual bool ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks) override;


	/**
	* Getters & Setters
	*/
public:
	void SetPlayerName(const string& name);
	inline const string& GetPlayerName() const { return m_playerName; }

	/** Is this controller owned by this client */
	inline bool IsLocal() const { return IsNetOwner(); }
};

