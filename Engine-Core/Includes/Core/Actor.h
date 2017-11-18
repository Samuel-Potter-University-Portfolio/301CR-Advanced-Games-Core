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


	/// Stores key bindings, so that the input controller may update them
	std::vector<struct KeyBinding*> m_keyBindings;


	// Location used for syncing back and forth between network
	vec2 m_netLocation;
	vec2 m_desiredLocation;
	bool bLocationUpdated = false;
	uint8 m_netLocationCheckId = 0; // Used to verify set location calls (Host always has precedence)


	/// RPCs to sync location
	void SendLocationToHost(const vec2& location, const uint8& checkId);
	void SendLocationToOwner(const vec2& location, const uint8& checkId);


protected:
	bool bIsTickable; 
	uint8 m_drawingLayer;

	/// How far behind the netlocation can this actor be before snapping back to it (For puppets)
	float m_catchupDistance = 5.0f;


public:
	AActor();

	/**
	* Callback for when actor gets loaded into level
	* @param level			The level that this actor is loading into
	*/
	void OnLevelLoaded(LLevel* level);

	/**
	* Callback for when this actor is loaded into the game as an active object
	*/
	virtual void OnBegin() override;
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

protected:
	/**
	* Internally registers this keybinding
	* @param binding		The binding for this key (Pointer must exist for duration of this actor)
	*/
	inline void RegisterKeybinding(KeyBinding* binding) { m_keyBindings.emplace_back(binding); }



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

	inline const std::vector<KeyBinding*>& GetKeyBindings() const { return m_keyBindings; }
	inline const bool& CanReceiveInput() const { return m_keyBindings.size(); }

	inline void SetLocation(const vec2& location) { m_desiredLocation = location; bLocationUpdated = true; }
	inline void Translate(const vec2& amount) { m_desiredLocation += amount; bLocationUpdated = true; }
	inline const vec2& GetLocation() const { return m_desiredLocation; }

	inline LLevel* GetLevel() const { return m_level; }
	class AssetController* GetAssetController();

	/** Was this actor create with the level initially (i.e. has a level switch unique instance id)*/
	inline const bool& WasSpawnedWithLevel() const { return bWasSpawnedWithLevel; }

	inline const uint8& GetDrawingLayer() const { return m_drawingLayer; }
};

