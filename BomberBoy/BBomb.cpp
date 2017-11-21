#include "BBomb.h"
#include "BLevelArena.h"


CLASS_SOURCE(ABBomb)


ABBomb::ABBomb() :
	m_drawSize(vec2(32.0f, 32.0f)),
	m_drawOffset(vec2(5.0f, 4.0f))
{
	bIsNetSynced = true;
	bIsTickable = true;
	m_drawingLayer = 2;

	m_movementSpeed = 0.25f;
	SetActive(false); // Start in active
}

bool ABBomb::RegisterRPCs(const char* func, RPCInfo& outInfo) const 
{
	RPC_INDEX_HEADER(func, outInfo);
	RPC_INDEX(UDP, RPCCallingMode::Broadcast, SyncAnimation);
	return false;
}
bool ABBomb::ExecuteRPC(uint16& id, ByteBuffer& params)
{
	RPC_EXEC_HEADER(id, params);
	RPC_EXEC(SyncAnimation);
	return false;
}


void ABBomb::RegisterAssets(Game* game) 
{
	AssetController* assets = game->GetAssetController();
	game->RegisterClass(StaticClass());

	// Don't register textures and animations for server
#ifdef BUILD_CLIENT
	static const string skins[]
	{
		"Default"
	};


	// Import all player directions (And setup animations)
	for (const string& skin : skins)
	{
		// Split the bomb animation atlas out
		// Register each bomb texture under Resources\Items\<skin>_Bomb.png.<frame>
		{
			sf::Image atlas;
			atlas.loadFromFile("Resources\\Items\\" + skin + "_Bomb.png");
			const uint32 frameSize = atlas.getSize().y;
			for (uint32 i = 0; i < 2; ++i)
			{
				sf::Image subimage;
				subimage.create(frameSize, frameSize, sf::Color(1, 1, 1));
				uint32 x = i;
				uint32 y = 0;
				subimage.copy(atlas, 0, 0, sf::IntRect::Rect(x * frameSize, y * frameSize, frameSize, frameSize), false);

				sf::Texture* texture = new sf::Texture;
				texture->loadFromImage(subimage);
				texture->setRepeated(true);
				texture->setSmooth(false);
				assets->RegisterTexture("Resources\\Items\\" + skin + "_Bomb.png." + std::to_string(i), texture);
			}
		}
		

		// Setup animation
		AnimationSheet* anim = new AnimationSheet;
		anim->SetFrameDuration(0.05f);
		anim->AddFrame(game->GetAssetController()->GetTexture("Resources\\Items\\" + skin + "_Bomb.png.0"));
		anim->AddFrame(game->GetAssetController()->GetTexture("Resources\\Items\\" + skin + "_Bomb.png.1"));
		assets->RegisterAnimation("Resources\\Items\\" + skin + "_Bomb.anim", anim);
	}
#endif
}

void ABBomb::OnBegin() 
{
	Super::OnBegin();
	m_animation = GetAssetController()->GetAnimation("Resources\\Items\\Default_Bomb.anim");
}

void ABBomb::OnTick(const float& deltaTime) 
{
	Super::OnTick(deltaTime);

	if (m_explodeTimer > 0.0f)
	{
		m_explodeTimer -= deltaTime;
		if (m_explodeTimer < 0.0f)
			m_explodeTimer = 0.0f;
	}
	else
	{
		// TODO - Do explosion logic
		if (IsNetHost())
		{
			SetActive(false);
			GetArena()->SetTile(GetTileLocation().x, GetTileLocation().y, ABLevelArena::TileType::Floor);
		}
	}
}

#ifdef BUILD_CLIENT
void ABBomb::OnDraw(sf::RenderWindow* window, const float& deltaTime) 
{
	sf::RectangleShape rect;
	rect.setSize(m_drawSize);

	// Draw pulsing bomb
	if (m_explodeTimer > 0.0f)
	{
		const float t = m_explodeTimer / m_explodeLength;

		// Pulse larger when closer to exploding
		const float pulseFrequency = 10.0f;
		const float pulseScale = 1.0f * (t) + 30.0f * (1.0f - t);

		const vec2 pulseSize = vec2(pulseScale, pulseScale) * std::abs(std::sin(t * 3.141592f * pulseFrequency));

		rect.setPosition(GetLocation() + m_drawOffset - pulseSize * 0.5f);
		rect.setSize(m_drawSize + pulseSize);

		if (m_animation != nullptr)
			rect.setTexture(m_animation->GetCurrentFrame());


		// Blur to grey when close to explosion
		if (t < 0.05f)
		{
			const uint8 v = (uint8)((t / 0.05f) * 255);
			rect.setFillColor(sf::Color(v,v,v));
		}
		window->draw(rect);
	}
}
#endif

bool ABBomb::AttemptToPlace(const ivec2& location)
{
	// Already in use/ Don't have permission
	if (IsActive() || !IsNetHost())
		return false;

	// Can only place on floor
	if (GetArena()->GetTile(location.x, location.y) == ABLevelArena::TileType::Floor) 
	{
		GetArena()->SetTile(location.x, location.y, ABLevelArena::TileType::Actor);
		SetTileLocation(location);

		m_explodeTimer = m_explodeLength;
		m_damageTimer = m_damageLength;
		SetActive(true);
		CallRPC(this, SyncAnimation);
	}
}

void ABBomb::SyncAnimation() 
{
	m_explodeTimer = m_explodeLength;
	m_damageTimer = m_damageLength;
}