#pragma once
#include "Core\Core-Common.h"
#include "BTileableActor.h"

#include "Core\Camera.h"
#include "BBomb.h"



/**
* Represents player characters used in BomberBoy
*/
class ABCharacter : public ABTileableActor
{
	CLASS_BODY()
	friend class ABMatchController;
public:
	static const uint32 s_maxBombCount;

private:
	///
	/// Score vars
	///
	bool bIsDead = true;
	int32 m_kills = 0;
	int32 m_deaths = 0;
	int32 m_bombsPlaced = 0;
	int32 m_roundWins = 0;


	///
	/// Visual vars
	///
	uint16 m_colourIndex = 16;
	const AnimationSheet* m_animUp = nullptr;
	const AnimationSheet* m_animDown = nullptr;
	const AnimationSheet* m_animLeft = nullptr;
	const AnimationSheet* m_animRight = nullptr;

	///
	/// Control vars
	///
	class OBPlayerController* m_playerController = nullptr;
	ACamera* m_camera;
	const vec2 m_drawSize;
	const vec2 m_drawOffset;


	///
	/// Control bindings
	///
	KeyBinding m_upKey;
	KeyBinding m_downKey;
	KeyBinding m_leftKey;
	KeyBinding m_rightKey;
	KeyBinding m_bombKey;


	/// Pool bombs
	std::vector<ABBomb*> m_bombs;
	/// Get a bomb to use
	inline ABBomb* GetNewBomb() const 
	{ 
		for (ABBomb* bomb : m_bombs)
			if (!bomb->IsActive())
				return bomb;
		return nullptr;
	}

public:
	ABCharacter();

	virtual void OnBegin() override;
	virtual void OnDestroy() override;

	/**
	* Registers the needed assets for ABCharacter
	* @param game			Where to register the assets
	*/
	static void RegisterAssets(Game* game);


	virtual void OnTick(const float& deltaTime) override;
#ifdef BUILD_CLIENT
	virtual void OnDraw(sf::RenderWindow* window, const float& deltaTime) override;
#endif

	/**
	* Attempt to place a bomb at this player's feet
	* @param tile		The tile to place it on
	*/
	void PlaceBomb(const ivec2& tile);

	/**
	* Update the character's animations to make sure the colours are correct
	* @param colourIndex			The index to use as a colour
	*/
	void SetColour(const uint16& colourIndex);

	/**
	* Lets the level controller spawn this character in at this tile
	* (Only callable by host)
	* @param tile			The tile to spawn on
	*/
	void SpawnAtTile(const ivec2& tile);

	/**
	* Net overrides
	*/
protected:
	virtual bool RegisterRPCs(const char* func, RPCInfo& outInfo) const override;
	virtual bool ExecuteRPC(uint16& id, ByteBuffer& params) override;

	virtual void RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode) override;
	virtual bool ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks) override;

private:
	inline void OnChange_ColourIndex();
	inline void OnSpawn(const ivec2& tile) { SetTileLocation(tile); }
};


typedef ABCharacter* ABCharacterPtr;
template<>
inline void Encode<ABCharacter*>(ByteBuffer& buffer, const ABCharacterPtr& data)
{
	Encode<AActor*>(buffer, data);
}

template<>
inline bool Decode<ABCharacter*>(ByteBuffer& buffer, ABCharacterPtr& out, void* context)
{
	AActor* actor;
	if (!Decode<AActor*>(buffer, actor, context)) return false;
	out = dynamic_cast<ABCharacter*>(actor);
	return true;
}