#pragma once
#include "BTileableActor.h"


class ABBomb : public ABTileableActor
{
	CLASS_BODY()
private:
	friend class ABCharacter;
	ABCharacter* m_parent = nullptr;

	const vec2 m_drawSize;
	const vec2 m_drawOffset;
	const AnimationSheet* m_animation = nullptr;

	/// How long until this explodes
	float m_explodeTimer;
	/// How long to wait until exploding
	float m_explodeLength = 5.0f;

	/// How long (after the explosion) until going inactive
	float m_damageTimer;
	/// How long (after the explosion) this will remain before going inactive
	float m_damageLength = 1.0f;

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

	//virtual void RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode) override;
	//virtual bool ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks) override;


	/**
	* Attempt to place this bomb at this location (tile)
	* (Expects to be called by host)
	* @param location		The location of the tile to place this bomb at
	* @returns If successful
	*/
	bool AttemptToPlace(const ivec2& location);
	/**
	* Called when this bomb is placed in world 
	* (Lets clients sync up for animations
	*/
	void SyncAnimation();
};

