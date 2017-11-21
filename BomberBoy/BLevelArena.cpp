#include "BLevelArena.h"


CLASS_SOURCE(ABLevelArena)

static const string tileSetNames[]
{
	"Default"
};


ABLevelArena::ABLevelArena() : 
	m_tileSize(vec2(42.0f, 42.0f)),
	m_arenaSize(10, 10), 

	m_currentWallTiles({nullptr})
{
	m_drawingLayer = 1;
	bIsNetSynced = true;

	m_currentFloorTile = nullptr;
	m_currentBoxTile = nullptr;
	m_currentLootTile = nullptr;

	m_tiles.resize(m_arenaSize.x * m_arenaSize.y, TileType::Floor);
	m_tiles[GetTileIndex(7, 8)] = TileType::Box;
	m_tiles[GetTileIndex(8, 8)] = TileType::Box;
	m_tiles[GetTileIndex(7, 6)] = TileType::Loot;
	m_tiles[GetTileIndex(0, 0)] = TileType::Loot;


	m_tiles[GetTileIndex(4, 2)] = TileType::Wall;
	m_tiles[GetTileIndex(5, 3)] = TileType::Wall;
	m_tiles[GetTileIndex(5, 2)] = TileType::Wall;
	m_tiles[GetTileIndex(3, 2)] = TileType::Wall;
	m_tiles[GetTileIndex(4, 3)] = TileType::Wall;
	m_tiles[GetTileIndex(4, 1)] = TileType::Wall;
}


void ABLevelArena::RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode) 
{
	SYNCVAR_INDEX_HEADER(outQueue, socketType, index, trackIndex, forceEncode);
	SYNCVAR_INDEX(UDP, SyncVarMode::Interval, TileGrid, m_tiles);
}

bool ABLevelArena::ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks) 
{
	SYNCVAR_EXEC_HEADER(id, value, skipCallbacks);
	SYNCVAR_EXEC(m_tiles);
	return false;
}


void ABLevelArena::RegisterAssets(Game* game) 
{
	AssetController* assets = game->GetAssetController();
	game->RegisterClass(StaticClass());

	// Don't register textures and animations for server
#ifdef BUILD_CLIENT

	// Import all tilesets
	for (const string& name : tileSetNames)
	{
		assets->RegisterTexture("Resources\\Level\\" + name + "_Box.png", false, true);
		assets->RegisterTexture("Resources\\Level\\" + name + "_Loot.png", false, true);
		assets->RegisterTexture("Resources\\Level\\" + name + "_Floor.png", false, true);

		// Wall tiles are stored in an atlas
		// -Combinations are stored in binary order where 0000 means no wall tiles adjacent
		// -Right most digit refers to top wall tile then goes clockwise e.g. 1010 Means: Top empty, Right wall, Bottom empty, left wall

		// Stored as <name>_Walls.png.<num> when num is the decimal representation of the nibble
		sf::Image atlas;
		atlas.loadFromFile("Resources\\Level\\" + name + "_Walls.png");
		const uint32 tileWidth = atlas.getSize().x / 4;
		const uint32 tileHeight = atlas.getSize().y / 4;
		for (uint32 i = 0; i < 16; ++i)
		{
			sf::Image subimage;
			subimage.create(tileWidth, tileHeight, sf::Color(1, 1, 1));
			uint32 x = i % 4;
			uint32 y = i / 4;
			subimage.copy(atlas, 0, 0, sf::IntRect::Rect(x * tileWidth, y * tileHeight, tileWidth, tileHeight), true);

			sf::Texture* texture = new sf::Texture;
			texture->loadFromImage(subimage);
			texture->setRepeated(true);
			texture->setSmooth(false);
			assets->RegisterTexture("Resources\\Level\\" + name + "_Walls.png." + std::to_string(i), texture);
		}
	}

#endif
}


void ABLevelArena::OnBegin() 
{
	SetTileset(TileSet::Default);
	SetLocation(vec2(m_arenaSize.x * m_tileSize.x * -0.5f, m_arenaSize.y * m_tileSize.y * -0.5f));
}


#ifdef BUILD_CLIENT
void ABLevelArena::OnDraw(sf::RenderWindow* window, const float& deltaTime) 
{
	for (int x = 0; x < m_arenaSize.x; ++x)
		for (int y = 0; y < m_arenaSize.y; ++y)
		{
			const TileType& tile = GetTile(x, y);

			sf::RectangleShape tileRect;
			tileRect.setPosition(GetLocation() + vec2((x) * m_tileSize.x, (y) * m_tileSize.y)); // Draw from centre of tile
			tileRect.setSize(m_tileSize);
			

			// Draw special tiles
			switch (tile)
			{
				case TileType::Floor:
					tileRect.setTexture(m_currentFloorTile);
					window->draw(tileRect);
					break;

				case TileType::Box:
					// Draw floor under box
					tileRect.setTexture(m_currentFloorTile);
					window->draw(tileRect);

					tileRect.setTexture(m_currentBoxTile);
					window->draw(tileRect);
					break;

				case TileType::Loot:
					// Draw floor under loot
					tileRect.setTexture(m_currentFloorTile);
					window->draw(tileRect);

					tileRect.setTexture(m_currentLootTile);
					window->draw(tileRect);
					break;

				case TileType::Wall:
					uint32 tileId = 0;
					if (GetTile(x, y - 1) == TileType::Wall)
						tileId |= 1;
					if (GetTile(x + 1, y) == TileType::Wall)
						tileId |= 2;
					if (GetTile(x, y + 1) == TileType::Wall)
						tileId |= 4;
					if (GetTile(x - 1, y) == TileType::Wall)
						tileId |= 8;

					tileRect.setTexture(m_currentWallTiles[tileId]);
					window->draw(tileRect);
					break;
			}
		}
}
#endif

void ABLevelArena::SetTileset(const TileSet& set)
{
#ifdef BUILD_CLIENT
	m_currentFloorTile = GetAssetController()->GetTexture("Resources\\Level\\" + tileSetNames[set] + "_Floor.png");
	m_currentBoxTile = GetAssetController()->GetTexture("Resources\\Level\\" + tileSetNames[set] + "_Box.png");
	m_currentLootTile = GetAssetController()->GetTexture("Resources\\Level\\" + tileSetNames[set] + "_Loot.png");

	for (uint32 i = 0; i < 16; ++i)
		m_currentWallTiles[i] = GetAssetController()->GetTexture("Resources\\Level\\" + tileSetNames[set] + "_Walls.png." + std::to_string(i));
#endif
}