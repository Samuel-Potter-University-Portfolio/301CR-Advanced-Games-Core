#pragma once
#include "Core\Core-Common.h"
#include <array>


/**
* Manages all of the tiles and makes sure that actors don't leave the play area
*/
class ABLevelArena : public AActor
{
	CLASS_BODY()
public:
	/// Different tile sets that arenas can use
	enum TileSet
	{
		Default
	};

	/// What this current tile can possibly be
	enum TileType 
	{
		Floor = 0, 
		Wall, 
		Box, 
		Loot,

		Count // How many tile types there are
	};
	
private:
	ivec2 m_arenaSize;
	const vec2 m_tileSize;

	const sf::Texture* m_currentFloorTile;
	const sf::Texture* m_currentBoxTile;
	const sf::Texture* m_currentLootTile;
	std::array<const sf::Texture*, 16> m_currentWallTiles;


public:
	ABLevelArena();


	virtual void OnBegin() override;
	//virtual void OnDestroy() override;

	/**
	* Registers the needed assets for ABLevelArena
	* @param game			Where to register the assets
	*/
	static void RegisterAssets(Game* game);


	//virtual void OnTick(const float& deltaTime) override;
#ifdef BUILD_CLIENT
	virtual void OnDraw(sf::RenderWindow* window, const float& deltaTime) override;
#endif


	/**
	* Net overrides
	*/
protected:
	//virtual bool RegisterRPCs(const char* func, RPCInfo& outInfo) const override;
	//virtual bool ExecuteRPC(uint16& id, ByteBuffer& params) override;

	//virtual void RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode) override;
	//virtual bool ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks) override;


public:
	/**
	* Corrects an actors velocity, so that it will not collide
	* @param actor			The actor to correct
	* @param velocity		The velocity you would like to move by (Function will edit this, if invalid)
	* @returns If the arena had to do an correcting
	*/
	bool CorrectMovement(AActor* actor, vec2& velocity);

	/**
	* Set the current tile set for the arena to use
	* @param set		The desired set
	*/
	void SetTileset(const TileSet& set);


public:
	/** Get the extents of this arena */
	inline sf::FloatRect GetArenaExtents() const { const vec2& loc = GetLocation(); return sf::FloatRect(loc.x, loc.y, m_arenaSize.x * m_tileSize.x, m_arenaSize.y * m_tileSize.y); }

	/** Convert a world position to a tile position	*/
	inline ivec2 WorldToTile(const vec2& worldPosition) const { return ivec2(std::round((worldPosition.x - GetLocation().x) / m_tileSize.x), std::round((worldPosition.y - GetLocation().y) / m_tileSize.y)); }
	/** Convert a tile position to a world position	*/
	inline vec2 TileToWorld(const ivec2& tilePosition) const { return vec2(GetLocation().x + tilePosition.x * m_tileSize.x, GetLocation().y + tilePosition.y * m_tileSize.y); }

};

