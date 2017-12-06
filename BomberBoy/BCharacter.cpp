#include "BCharacter.h"
#include "BLevelArena.h"
#include "Utils.h"

#include "BPlayerController.h"
#include "BLevelController.h"


CLASS_SOURCE(ABCharacter)
const uint32 ABCharacter::s_maxBombCount = 10;


ABCharacter::ABCharacter() :
	m_drawSize(vec2(32.0f, 42.0f)),
	m_drawOffset(vec2(5.0f, 0.0f)),

	m_upKey(sf::Keyboard::Key::W),
	m_downKey(sf::Keyboard::Key::S),
	m_leftKey(sf::Keyboard::Key::A),
	m_rightKey(sf::Keyboard::Key::D),
	m_bombKey(sf::Keyboard::Key::Space)
{
	bIsNetSynced = true;
	bIsTickable = true;
	m_drawingLayer = 3;

	m_movementSpeed = 0.4f;

	RegisterKeybinding(&m_upKey);
	RegisterKeybinding(&m_downKey);
	RegisterKeybinding(&m_leftKey);
	RegisterKeybinding(&m_rightKey);
	RegisterKeybinding(&m_bombKey);

	m_bombs.reserve(s_maxBombCount);
}

bool ABCharacter::RegisterRPCs(const char* func, RPCInfo& outInfo) const 
{
	RPC_INDEX_HEADER(func, outInfo);
	RPC_INDEX(UDP, RPCCallingMode::Host, PlaceBomb);
	RPC_INDEX(TCP, RPCCallingMode::Broadcast, OnSpawn);
	return false;
}
bool ABCharacter::ExecuteRPC(uint16& id, ByteBuffer& params)
{
	RPC_EXEC_HEADER(id, params);
	RPC_EXEC_OneParam(PlaceBomb, ivec2);
	RPC_EXEC_OneParam(OnSpawn, ivec2);
	return false;
}

void ABCharacter::RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode) 
{
	SYNCVAR_INDEX_HEADER(outQueue, socketType, index, trackIndex, forceEncode);
	SYNCVAR_INDEX(UDP, uint16, m_colourIndex);
	SYNCVAR_INDEX(UDP, int32, m_kills);
	SYNCVAR_INDEX(UDP, int32, m_deaths);
	SYNCVAR_INDEX(UDP, int32, m_roundWins);
	SYNCVAR_INDEX(UDP, bool, bIsDead);
	SYNCVAR_INDEX(TCP, OBPlayerController*, m_playerController);
}
bool ABCharacter::ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks) 
{
	SYNCVAR_EXEC_HEADER(id, value, skipCallbacks);
	SYNCVAR_EXEC_AlwaysCallback(m_colourIndex, OnChange_ColourIndex);
	SYNCVAR_EXEC(m_kills);
	SYNCVAR_EXEC(m_deaths);
	SYNCVAR_EXEC(m_roundWins);
	SYNCVAR_EXEC(bIsDead);
	SYNCVAR_EXEC(m_playerController);
	return false;
}



void ABCharacter::RegisterAssets(Game* game) 
{
	AssetController* assets = game->GetAssetController();
	game->RegisterClass(StaticClass());

	ABBomb::RegisterAssets(game);

	// Don't register textures and animations for server
#ifdef BUILD_CLIENT
	static const string directions[]
	{
		"Up", "Down", "Left", "Right"
	};


	// Import all player directions (And setup animations)
	for (const string& dir : directions)
	{
		// Register textures
		for (uint32 i = 0; i < 3; ++i)
		{
			// Import default (Grey character)
			const string defaultPath = "Resources\\Character\\" + dir + "_" + std::to_string(i) + ".png";
			assets->RegisterTexture(defaultPath, false);

			// Register player/team colours
			// Asset gets registerd at path (Where path is xyz.png) xyz.png.<colour rgb value as string>
			for (const Colour& colour : OBPlayerController::s_supportedColours)
			{
				// Get texture in desired colour
				sf::Texture* texture = new sf::Texture;
				texture->setSmooth(false);
				texture->setRepeated(false);

				sf::Image image;
				image.loadFromFile(defaultPath);
				CastColourFromCommonGrey(image, colour);
				texture->loadFromImage(image);

				const string colourName = std::to_string(colour.r) + std::to_string(colour.g) + std::to_string(colour.b);
				assets->RegisterTexture(defaultPath + "." + colourName, texture);
			}
		}


		// Setup animation
		// Default (grey)
		AnimationSheet* anim = new AnimationSheet;
		anim->SetFrameDuration(0.15f);
		anim->AddFrame(game->GetAssetController()->GetTexture("Resources\\Character\\" + dir + "_0.png"));
		anim->AddFrame(game->GetAssetController()->GetTexture("Resources\\Character\\" + dir + "_1.png"));
		anim->AddFrame(game->GetAssetController()->GetTexture("Resources\\Character\\" + dir + "_0.png"));
		anim->AddFrame(game->GetAssetController()->GetTexture("Resources\\Character\\" + dir + "_2.png"));
		assets->RegisterAnimation("Resources\\Character\\" + dir + ".anim", anim);

		// Register player/team colours
		// Asset gets registerd at path (Where path is xyz.anim) xyz.anim.<colour name>
		for (const Colour& colour : OBPlayerController::s_supportedColours)
		{
			const string colourName = std::to_string(colour.r) + std::to_string(colour.g) + std::to_string(colour.b);
			AnimationSheet* colAnim = new AnimationSheet;
			colAnim->SetFrameDuration(0.15f);
			colAnim->AddFrame(game->GetAssetController()->GetTexture("Resources\\Character\\" + dir + "_0.png." + colourName));
			colAnim->AddFrame(game->GetAssetController()->GetTexture("Resources\\Character\\" + dir + "_1.png." + colourName));
			colAnim->AddFrame(game->GetAssetController()->GetTexture("Resources\\Character\\" + dir + "_0.png." + colourName));
			colAnim->AddFrame(game->GetAssetController()->GetTexture("Resources\\Character\\" + dir + "_2.png." + colourName));
			assets->RegisterAnimation("Resources\\Character\\" + dir + ".anim." + colourName, colAnim);
		}
	}
#endif
}


void ABCharacter::OnBegin()
{
	Super::OnBegin();

	if (IsNetHost())
	{
		// Setup bombs
		for (uint32 i = 0; i < s_maxBombCount; ++i)
		{
			ABBomb* bomb = GetLevel()->SpawnActor<ABBomb>(ABBomb::StaticClass(), this);
			bomb->m_parent = this;
			m_bombs.emplace_back(bomb);
		}
	}
	

#ifdef BUILD_CLIENT
	// Cache camera
	if (IsNetOwner())
	{
		m_camera = GetGame()->GetCurrentLevel()->GetFirstActor<ACamera>();
	}
	// Just to force set
	OnChange_ColourIndex();
#endif
}

void ABCharacter::OnDestroy() 
{
	Super::OnDestroy();

	// Cleanup bombs
	if(IsNetHost())
		for (ABBomb* bomb : m_bombs)
			Destroy(bomb);
}

void ABCharacter::OnTick(const float& deltaTime) 
{
	Super::OnTick(deltaTime);

	// Ignore calls if dead
	if (bIsDead)
		return;

	// Interaction
	if (IsNetOwner())
	{
		if (m_upKey.IsHeld())
			AttemptMove(Direction::Up);
		if (m_downKey.IsHeld())
			AttemptMove(Direction::Down);
		if (m_leftKey.IsHeld())
			AttemptMove(Direction::Left);
		if (m_rightKey.IsHeld())
			AttemptMove(Direction::Right);

		if (m_bombKey.IsHeld())
			CallRPC_OneParam(this, PlaceBomb, GetClosestTileLocation());


		// Move camera
		const vec2 diff = m_camera->GetLocation() - GetLocation();
		const float sqrdDist = diff.x*diff.x + diff.y*diff.y;

		if (sqrdDist >= 100.0f * 100.0f)
			m_camera->SetLocation(m_camera->GetLocation() * 0.99f + GetLocation() * 0.01f);
	}

	// Check to see if dead or not
	if (IsNetHost())
	{
		const ivec2 tileLocation = GetTileLocation();
		ABLevelArena::TileType tile = GetArena()->GetTile(tileLocation.x, tileLocation.y);
		if (tile == ABLevelArena::TileType::Explosion)
		{
			ABMatchController* controller = dynamic_cast<ABMatchController*>(GetLevel()->GetLevelController());
			if (controller != nullptr)
				controller->OnPlayerExploded(this, GetArena()->GetExplosionOwner(tileLocation));
		}
	}
}

#ifdef BUILD_CLIENT
void ABCharacter::OnDraw(sf::RenderWindow* window, const float& deltaTime) 
{
	const Direction& direction = GetDirection();
	const AnimationSheet* anim =
		direction == Direction::Up ? m_animUp :
		direction == Direction::Down ? m_animDown :
		direction == Direction::Left ? m_animLeft :
		m_animRight;

	sf::RectangleShape rect;
	rect.setPosition(GetLocation() + m_drawOffset);
	rect.setSize(m_drawSize);
	if (anim != nullptr)
	{
		if(IsMoving())
			rect.setTexture(anim->GetCurrentFrame());
		else
			rect.setTexture(anim->GetFrame(0));
	}
	window->draw(rect);
}
#endif

void ABCharacter::PlaceBomb(const ivec2& tile)
{
	ABBomb* bomb = GetNewBomb();
	if (!bIsDead && bomb != nullptr && bomb->AttemptToPlace(tile))
		++m_bombsPlaced;
}

void ABCharacter::SetColour(const uint16& colourIndex)
{
	if (m_colourIndex != colourIndex)
	{
		m_colourIndex = colourIndex;
		OnChange_ColourIndex();
	}
}

void ABCharacter::OnChange_ColourIndex()
{
#ifdef BUILD_CLIENT
	const Colour colour = OBPlayerController::s_supportedColours[m_colourIndex];
	const string colourName = std::to_string(colour.r) + std::to_string(colour.g) + std::to_string(colour.b);

	// Load default animations
	m_animUp = GetGame()->GetAssetController()->GetAnimation("Resources\\Character\\Up.anim." + colourName);
	m_animDown = GetGame()->GetAssetController()->GetAnimation("Resources\\Character\\Down.anim." + colourName);
	m_animLeft = GetGame()->GetAssetController()->GetAnimation("Resources\\Character\\Left.anim." + colourName);
	m_animRight = GetGame()->GetAssetController()->GetAnimation("Resources\\Character\\Right.anim." + colourName);
#endif
}

void ABCharacter::SpawnAtTile(const ivec2& tile) 
{
	if (IsNetHost())
	{
		OnSpawn(tile);
		bIsDead = false;
		CallRPC_OneParam(this, OnSpawn, tile);
	}
}