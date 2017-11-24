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
	enum TileSet : uint8
	{
		Default
	};

	/// What this current tile can possibly be
	enum TileType : uint8
	{
		Unknown = 0,

		Floor, 
		Wall, 
		Box, 
		LootBox,

		Bomb,			// Bomb currently on the ground
		Explosion,		// Explosion tile (That players take damage from)s
	};

	typedef std::vector<ABLevelArena::TileType> TileGrid;
	
private:
	uvec2 m_arenaSize;
	const vec2 m_tileSize;

	/// What tiles are placed where
	TileGrid m_tiles;
	/// What bombs are currently affecting which tiles
	std::vector<class ABBomb*> m_explosionParents;
	
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

	virtual void RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode) override;
	virtual bool ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks) override;


public:
	/**
	* Set the current tile set for the arena to use
	* @param set		The desired set
	*/
	void SetTileset(const TileSet& set);

	/**
	* Callback for when a bomb is placed in the arena
	* @param bomb		The bomb that we're trying to place
	*/
	void OnPlaceBomb(class ABBomb* bomb);
	/**
	* Callback for when a bomb's effects should disappear
	* @param bomb		The bomb that we're trying to get rid of
	*/
	void OnDestroyBomb(class ABBomb* bomb);

	/**
	* Handle a bomb explosion
	* @param bomb		The bomb that we're trying to explode
	*/
	void HandleExplosion(class ABBomb* bomb);

	/**
	* Getters & Setters
	*/
protected:
	inline uint32 GetTileIndex(const uint32& x, const uint32& y) const { return y * m_arenaSize.x + x; }
	inline bool SetTile(const uint32& x, const uint32& y, const TileType& tile)
	{
		if (x < 0 || y < 0 || x >= m_arenaSize.x || y >= m_arenaSize.y)
			return false;
		else
			m_tiles[GetTileIndex(x, y)] = tile;
		return true;
	}
public:
	inline TileType GetTile(const uint32& x, const uint32& y) { if (x < 0 || y < 0 || x >= m_arenaSize.x || y >= m_arenaSize.y) return TileType::Unknown; else return m_tiles[GetTileIndex(x, y)]; }
	


	/** Get the extents of this arena */
	inline sf::FloatRect GetArenaExtents() const { const vec2& loc = GetLocation(); return sf::FloatRect(loc.x, loc.y, m_arenaSize.x * m_tileSize.x, m_arenaSize.y * m_tileSize.y); }
	inline const vec2& GetTileSize() const { return m_tileSize; }

	/** Convert a world position to a tile position	*/
	inline ivec2 WorldToTile(const vec2& worldPosition) const { return ivec2((worldPosition.x - GetLocation().x) / m_tileSize.x, (worldPosition.y - GetLocation().y) / m_tileSize.y); }
	/** Convert a tile position to a world position	*/
	inline vec2 TileToWorld(const ivec2& tilePosition) const { return vec2(GetLocation().x + tilePosition.x * m_tileSize.x, GetLocation().y + tilePosition.y * m_tileSize.y); }
};



template<>
inline void Encode<ABLevelArena::TileGrid>(ByteBuffer& buffer, const ABLevelArena::TileGrid& data)
{
	// ~65000 is hard tile limit
	Encode<uint16>(buffer, (uint16)data.size());

	ABLevelArena::TileType current = ABLevelArena::TileType::Wall;
	uint16 count = 0;

	// Encode in format count,tile type ...
	for (uint32 i = 0; i < data.size(); ++i)
	{
		const ABLevelArena::TileType& tile = data[i];
		if (current == tile)
			++count;
		else
		{
			Encode<uint16>(buffer, count);
			Encode<uint8>(buffer, (uint8)current);

			current = tile;
			count = 1;
		}
	}

	// Encode latest
	Encode<uint16>(buffer, count);
	Encode<uint8>(buffer, (uint8)current);
}
template<>
inline bool Decode<ABLevelArena::TileGrid>(ByteBuffer& buffer, ABLevelArena::TileGrid& out, void* context)
{
	uint16 size;
	if (!Decode<uint16>(buffer, size))
		return false;
	out.resize(size);

	uint16 count;
	uint8 tileType;
	uint32 i = 0;


	while (size != 0)
	{
		if (!Decode<uint16>(buffer, count) || !Decode<uint8>(buffer, tileType))
			return false;

		const ABLevelArena::TileType tile = (ABLevelArena::TileType)tileType;

		for (uint32 c = 0; c < count; ++c, ++i)
			out[i] = tile;

		size -= count;
	}

	return true;
}