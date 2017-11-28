#pragma once
#include "Core\Core-Common.h"
#include "BCharacter.h"

#include <queue>




/**
* Player controller that is used for all BomberBoy gamemodes
*/
class OBPlayerController : public OPlayerController
{
	CLASS_BODY()
	friend class ABLevelControllerBase;
public:
	static const std::vector<Colour> s_supportedColours;
	static std::queue<uint32> s_colourQueue; // Currently avaliable colour indices

private:
	uint32 m_colourIndex = 16;
	bool bIsReady = false;

public:
	OBPlayerController();

	virtual void OnBegin() override;
	virtual void OnDestroy() override;

	/**
	* RPC and Net var overrides
	*/
protected:
	virtual bool RegisterRPCs(const char* func, RPCInfo& outInfo) const override;
	virtual bool ExecuteRPC(uint16& id, ByteBuffer& params) override;

	virtual void RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode) override;
	virtual bool ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks) override;


public:
	/**
	* Send chat message to the server
	*/
	void SendMessage(const string& message);

	/**
	* Update the player's state to ready
	*/
	inline void SetReady(const bool& value) { if (IsNetOwner()) CallRPC_OneParam(this, SetReady_Host, value); }

	/**
	* Update which map the player has voted for
	* @param index		The index of the map
	*/
	inline void SetMapVote(const uint32& index) { if (IsNetOwner()) CallRPC_OneParam(this, SetMapVote_Host, index); }

private:

	/**
	* Broadcast a chat message to all clients
	*/
	void BroadcastMessage(const string& message);

	inline void SetReady_Host(const bool& value) { bIsReady = value; }
	void SetMapVote_Host(const uint32& index);



	/**
	* Getters & Setters
	*/
public:
	/**
	* Retrieve a clamped user name that is safe for display
	*/
	string GetDisplayName() const;
	inline const bool& IsReady() const { return bIsReady; }

	inline uint32 GetColourIndex() const { return m_colourIndex; }
	inline Colour GetColour() const { return s_supportedColours[m_colourIndex]; }
	inline string GetColourCode() const { Colour c = GetColour(); return std::to_string(c.r) + std::to_string(c.g) + std::to_string(c.b); }
};


typedef OBPlayerController* OBPlayerControllerPtr;
template<>
inline void Encode<OBPlayerController*>(ByteBuffer& buffer, const OBPlayerControllerPtr& data)
{
	Encode<OObject*>(buffer, data);
}

template<>
inline bool Decode<OBPlayerController*>(ByteBuffer& buffer, OBPlayerControllerPtr& out, void* context)
{
	OObject* object;
	if (!Decode<OObject*>(buffer, object, context)) return false;
	out = dynamic_cast<OBPlayerController*>(object);
	return true;
}
