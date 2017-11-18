#pragma once
#include "Object.h"
#include <SFML/Graphics.hpp>


class LLevel;


/**
* Actors represent non-persistent types that can be spawned into a level
* All children of this class should append an A to the beginning of their name
*/
class CORE_API AActor : public OObject
{
	CLASS_BODY()
	friend LLevel;
private:
	static uint32 s_instanceCounter;
	const uint32 m_instanceId;
	LLevel* m_level = nullptr;

	bool bWasSpawnedWithLevel;


	// Location used for syncing back and forth between network
	vec2 m_netLocation;
	vec2 m_location;

	/// Callback for when net location is updated
	void OnNetLocationUpdate();


protected:
	bool bIsTickable; 
	uint8 m_drawingLayer;

	/// How far behind the netlocation can this actor be before snapping back to it
	float m_catchupDistance = 2.0f;

public:
	AActor();

	/**
	* Callback for when actor gets loaded into level
	* @param level			The level that this actor is loading into
	*/
	void OnLevelLoaded(LLevel* level);

	/**
	* Callback for main logic loop tick
	* @param deltaTime		Time since last tick in seconds
	*/
	virtual void OnTick(const float& deltaTime) {}

	/**
	* Callback for when after a tick as completed
	*/
	void OnPostTick();

#ifdef BUILD_CLIENT
	/**
	* Called when this actor should be drawn to the screen
	* @param window			The window to draw to
	* @param deltaTime		Time since last draw in seconds)
	* @param layer			The layer that is currently being drawn (From 0-10, where 10 is the last drawn/on top)
	*/
	virtual void OnDraw(sf::RenderWindow* window, const float& deltaTime) {}
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
	* Getters & Setters
	*/
public:
	/** A unique id applied to each object */
	inline const uint32& GetInstanceID() const { return m_instanceId; }

	inline const bool& IsTickable() const { return bIsTickable; }
	inline const bool& IsVisible() const { return true; }

	inline void SetLocation(const vec2& location) { m_location = location; }
	inline void Translate(const vec2& amount) { m_location += amount; }
	inline const vec2& GetLocation() const { return m_location; }

	inline LLevel* GetLevel() const { return m_level; }
	class AssetController* GetAssetController();

	/** Was this actor create with the level initially (i.e. has a level switch unique instance id)*/
	inline const bool& WasSpawnedWithLevel() const { return bWasSpawnedWithLevel; }

	inline const uint8& GetDrawingLayer() const { return m_drawingLayer; }
};

