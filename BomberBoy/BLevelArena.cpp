#include "BLevelArena.h"
#include "BBomb.h"


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
	m_explosionParents.resize(m_arenaSize.x * m_arenaSize.y, nullptr);
}


void ABLevelArena::RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode) 
{
	SYNCVAR_INDEX_HEADER(outQueue, socketType, index, trackIndex, forceEncode);
	SYNCVAR_INDEX(UDP, SyncVarMode::Always, TileGrid, m_tiles);
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
	const vec2 viewCentre = window->getView().getCenter();
	const vec2 viewHalfSize = window->getView().getSize() * 0.5f;

	const vec2 min = viewCentre - viewHalfSize;
	const vec2 max = viewCentre + viewHalfSize;


	for (int x = 0; x < m_arenaSize.x; ++x)
		for (int y = 0; y < m_arenaSize.y; ++y)
		{
			const TileType& tile = GetTile(x, y);
			const vec2 location = GetLocation() + vec2((x)* m_tileSize.x, (y)* m_tileSize.y);
			const vec2 halfSize = m_tileSize * 0.5f;

			// Cull shapes off screen
			if (location.x + halfSize.x < min.x || location.y + halfSize.y < min.y || location.x - halfSize.x > max.x || location.y - halfSize.y > max.y)
				continue;

			sf::RectangleShape tileRect;
			tileRect.setPosition(location); // Draw from centre of tile
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

				case TileType::LootBox:
					// Draw floor under loot
					tileRect.setTexture(m_currentFloorTile);
					window->draw(tileRect);

					tileRect.setTexture(m_currentLootTile);
					window->draw(tileRect);
					break;

				case TileType::Bomb:
					// Actor will draw itself, but will still need the floor to be there
					tileRect.setTexture(m_currentFloorTile);
					window->draw(tileRect);
					break;

				case TileType::Explosion:
					// TODO - Draw explosion
					tileRect.setFillColor(sf::Color::Red);
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

void ABLevelArena::ResetArena(uvec2 size)
{
	m_arenaSize = size;

	m_tiles.clear();
	m_tiles.resize(m_arenaSize.x * m_arenaSize.y, TileType::Floor);
	m_explosionParents.clear();
	m_explosionParents.resize(m_arenaSize.x * m_arenaSize.y, nullptr);

	for (uint32 x = 0; x < size.x; ++x)
		for (uint32 y = 0; y < size.y; ++y)
			if (x == 0 || y == 0 || x == size.x - 1 || y == size.y - 1)
				SetTile(x, y, TileType::Wall);


	const uint32 stride = (m_arenaSize.x - 2 - 1) / 3;
	m_spawnPoints.clear();
	for (uint32 i = 0; i < 4; ++i)
	{
		m_spawnPoints.emplace_back(1 + i * stride, 1);
		m_spawnPoints.emplace_back(1 + i * stride, m_arenaSize.x - 3);
	}
}

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

void ABLevelArena::OnPlaceBomb(ABBomb* bomb)
{
	const ivec2 location = bomb->GetTileLocation();
	const uint32 index = GetTileIndex(location.x, location.y);

	m_explosionParents[index]	= bomb;
	m_tiles[index]				= TileType::Bomb;
}	

void ABLevelArena::OnDestroyBomb(ABBomb* bomb)
{
	for (uint32 i = 0; i < m_tiles.size(); ++i)
	{
		// Reset references and tiles
		if (m_explosionParents[i] == bomb)
		{
			m_explosionParents[i]	= nullptr;
			m_tiles[i]				= TileType::Floor;
		}
	}
}

void ABLevelArena::HandleExplosion(ABBomb* bomb)
{
	// Use closest tile (As bomb may be in motion)
	const ivec2 centre = bomb->GetTileLocation();

	// Set all tiles to explosion
	const uint32 size = bomb->m_explosionSize;


	/// Attempt to explode this tile
	/// @returns If explosion should continue to grow
	auto handleExplodeTile =
	[this, bomb](const uint32& x, const uint32& y) -> bool 
	{
		const ABLevelArena::TileType& tile = GetTile(x, y);
		const uint32 index = GetTileIndex(x, y);
		switch (tile)
		{
			case ABLevelArena::TileType::Floor:
				SetTile(x, y, ABLevelArena::TileType::Explosion);
				m_explosionParents[index] = bomb;
				return true; // Pass through

			case ABLevelArena::TileType::Explosion:
				return true; // Pass through


			// Exlode bomb
			case ABLevelArena::TileType::Bomb:
			{
				ABBomb* tileBomb = m_explosionParents[index];
				if (tileBomb != nullptr)
					tileBomb->Explode();
				return false;
			}

			// Destroy box
			case ABLevelArena::TileType::Box:
				SetTile(x, y, ABLevelArena::TileType::Explosion);
				m_explosionParents[index] = bomb;
				return false;

			// Destroy loot box
			case ABLevelArena::TileType::LootBox:
				SetTile(x, y, ABLevelArena::TileType::Explosion);
				m_explosionParents[index] = bomb;
				// TODO - Drop loot
				return false;

			default:
				return false;
		}
	};

	// Explode this tile
	SetTile(centre.x, centre.y, ABLevelArena::TileType::Floor);
	handleExplodeTile(centre.x, centre.y);

	// Explode right line
	for (uint32 i = 1; i <= size; ++i)
		if (!handleExplodeTile(centre.x + i, centre.y))
			break;
	// Explode left line
	for (uint32 i = 1; i <= size; ++i)
		if (!handleExplodeTile(centre.x - i, centre.y))
			break;
	// Explode up line
	for (uint32 i = 1; i <= size; ++i)
		if (!handleExplodeTile(centre.x, centre.y - i))
			break;
	// Explode down line
	for (uint32 i = 1; i <= size; ++i)
		if (!handleExplodeTile(centre.x, centre.y + i))
			break;
}