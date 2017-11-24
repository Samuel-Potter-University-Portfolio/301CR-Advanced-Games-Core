#pragma once
#include "BTileableActor.h"


class ABBomb : public ABTileableActor
{
	CLASS_BODY()
private:
	friend class ABCharacter;
	friend class ABLevelArena;
	ABCharacter* m_parent = nullptr;

	const vec2 m_drawSize;
	const vec2 m_drawOffset;
	const AnimationSheet* m_animation = nullptr;

	/// How long until this explodes
	float m_explodeTimer;
	/// How long to wait until exploding
	const float m_explodeLength = 4.0f;

	/// How long (after the explosion) until going inactive
	float m_damageTimer;
	/// How long (after the explosion) this will remain before going inactive
	const float m_damageLength = 0.5f;

	/// How many tiles from the source this bomb will travel when exploding
	uint32 m_explosionSize = 1;
	/// Has this bomb exploded yet
	bool bHasExploded = false;

public:
	ABBomb();

	virtual void OnBegin() override;
	//virtual void OnDestroy() override;

	/**
	* Registers the needed assets for ABBomb
	* @param game			Where to register the assets
	*/
	static void RegisterAssets(Game* game);


	virtual void OnTick(const float& deltaTime) override;
	//virtual void OnTick(const float& deltaTime) override;
#ifdef BUILD_CLIENT
	virtual void OnDraw(sf::RenderWindow* window, const float& deltaTime) override;
#endif


	/**
	* Net sync functions
	*/
protected:
	virtual bool RegisterRPCs(const char* func, RPCInfo& outInfo) const override;
	virtual bool ExecuteRPC(uint16& id, ByteBuffer& params) override;

	virtual void RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode) override;
	virtual bool ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks) override;


	/**
	* Attempt to place this bomb at this location (tile)
	* (Expects to be called by host)
	* @param location		The location of the tile to place this bomb at
	* @returns If successful
	*/
	bool AttemptToPlace(const ivec2& location);

protected:
	/**
	* Causes this bomb to explode at it's current location
	* @returns If this call was successful
	*/
	bool Explode();
	/**
	* Called when this bomb is placed in world 
	* (Lets clients sync up for animations
	*/
	void SyncAnimation();
};

