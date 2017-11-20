#include "BLevelArena.h"


CLASS_SOURCE(ABLevelArena)

static const string tileSetNames[]
{
	"Default"
};


ABLevelArena::ABLevelArena() : 
	m_tileSize(vec2(20.0f, 20.0f) * 2.0f),
	m_arenaSize(10, 10), 

	m_currentWallTiles({nullptr})
{
	m_drawingLayer = 1;
	bIsNetSynced = true;

	m_currentFloorTile = nullptr;
	m_currentBoxTile = nullptr;
	m_currentLootTile = nullptr;
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
			texture->setSmooth(true);
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

			sf::RectangleShape tileRect;
			tileRect.setPosition(GetLocation() + vec2((x + 0.5f) * m_tileSize.x, (y + 0.5f) * m_tileSize.y)); // Draw from centre of tile
			tileRect.setSize(m_tileSize);

			// Draw floor
			if (m_currentFloorTile != nullptr)
			{
				tileRect.setTexture(m_currentFloorTile);
				window->draw(tileRect);
			}

			// Draw wall
			if (m_currentBoxTile != nullptr && (x == 0 || y == 0 || x == m_arenaSize.x - 1 || y == m_arenaSize.y - 1))
			{
				tileRect.setTexture(m_currentBoxTile);
				window->draw(tileRect);
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

bool ABLevelArena::CorrectMovement(AActor* actor, vec2& velocity) 
{
	const sf::FloatRect arenaExtents = GetArenaExtents();

	vec2 actorLocation = actor->GetLocation();
	const float actorRadius = m_tileSize.x * 0.5f;


	// Make sure player stays in play area and doesn't intersect any walls
	if (actor->HasNetControl())
	{
		// Arena extent check
		bool outOfBounds = false;

		if (actorLocation.x - actorRadius < arenaExtents.left)
		{
			const float desiredVel = (arenaExtents.left + actorRadius) - actorLocation.x;
			actorLocation.x = arenaExtents.left + actorRadius;
			velocity.x = velocity.x < desiredVel ? velocity.x : desiredVel;
			outOfBounds = true;
		}
		if (actorLocation.x + actorRadius > arenaExtents.left + arenaExtents.width)
		{
			const float desiredVel = (arenaExtents.left + arenaExtents.width - actorRadius) - actorLocation.x;
			actorLocation.x = arenaExtents.left + arenaExtents.width - actorRadius;
			velocity.x = velocity.x > desiredVel ? velocity.x : desiredVel;
			outOfBounds = true;
		}

		if (actorLocation.y - actorRadius < arenaExtents.top)
		{
			const float desiredVel = (arenaExtents.top + actorRadius) - actorLocation.y;
			actorLocation.y = arenaExtents.top + actorRadius;
			velocity.y = velocity.y < desiredVel ? velocity.y : desiredVel;
			outOfBounds = true;
		}
		if (actorLocation.y + actorRadius > arenaExtents.top + arenaExtents.height)
		{
			const float desiredVel = (arenaExtents.top + arenaExtents.height - actorRadius) - actorLocation.y;
			actorLocation.y = arenaExtents.top + arenaExtents.height - actorRadius;
			velocity.y = velocity.y > desiredVel ? velocity.y : desiredVel;
			outOfBounds = true;
		}

		if (outOfBounds)
		{
			actor->SetLocation(actorLocation);
			return true;
		} 
		
		
		// Tile checks
		{
			ivec2 startTile = WorldToTile(actor->GetLocation());


		}
	}


	

	return false;
}