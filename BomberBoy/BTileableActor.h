#pragma once
#include "Core\Core-Common.h"


/**
* Represents and actor which exists at at tile and can move between tiles
*/
class ABTileableActor : public AActor
{
	CLASS_BODY()
public:
	enum Direction : uint8
	{
		Up, Down, Left, Right
	};
private:
	/// The arena that this actor exists in
	class ABLevelArena* m_arena = nullptr;

	/// Where this actor exists
	ivec2 m_tileLocation;


	/// Is the actor currently moving
	bool bIsMoving;
	bool bNetIsMoving;

	/// How long until this actor can move again
	float m_movementCooldown;

	/// Current actor direction
	Direction m_direction;
	Direction m_netDirection;

protected:
	/// How long it takes (In seconds) for 1 movement
	float m_movementSpeed = 1.0f;

	inline ABLevelArena* GetArena() const { return m_arena; }
	 
public:
	ABTileableActor();

	virtual void OnBegin() override;
	virtual void OnTick(const float& deltaTime) override;
	virtual void OnPostNetInitialize() override;

	/**
	* Attempt to move this actor 
	* @param dir		The direction to move
	* @returns Whether this call was successful or not
	*/
	bool AttemptMove(const Direction& dir);


	/**
	* Net overrides
	*/
protected:
	virtual bool RegisterRPCs(const char* func, RPCInfo& outInfo) const override;
	virtual bool ExecuteRPC(uint16& id, ByteBuffer& params) override;

	virtual void RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode) override;
	virtual bool ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks) override;

	/**
	* Update the server with the current states of direction and movement
	* @param direction			The direction the actor is looking
	* @param isMoving			Is the actor currently moving
	*/
	inline void UpdateNetMoveState(const Direction& direction, const bool& isMoving) 
	{
		bNetIsMoving = isMoving;
		m_netDirection = direction;
	}

	/**
	* Getters & Setters
	*/
public:
	inline const bool& IsMoving() const { return IsNetOwner() ? bIsMoving : bNetIsMoving; }
	inline const Direction& GetDirection() const { return IsNetOwner() ? m_direction : m_netDirection; }

	inline const ivec2& GetTileLocation() const { return m_tileLocation; }
	void SetTileLocation(const ivec2& tile);
	
	/**
	* Get the tile closest to this player e.g. if moving, get destination, if that is closer
	*/
	ivec2 GetClosestTileLocation() const;
};


template<>
inline void Encode<ABTileableActor::Direction>(ByteBuffer& buffer, const ABTileableActor::Direction& data)
{
	Encode<uint8>(buffer, (uint8)data);
}
template<>
inline bool Decode<ABTileableActor::Direction>(ByteBuffer& buffer, ABTileableActor::Direction& out, void* context)
{
	uint8 raw;
	if (!Decode<uint8>(buffer, raw))
		return false;
	out = (ABTileableActor::Direction)raw;
	return true;
}
