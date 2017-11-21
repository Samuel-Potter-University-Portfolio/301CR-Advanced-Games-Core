#include "BCharacter.h"
#include "BLevelArena.h"
#include "Utils.h"


CLASS_SOURCE(ABCharacter)


const std::vector<CharacterColour> ABCharacter::s_supportedColours(
{
	CharacterColour("Red", sf::Color::Red),
	CharacterColour("Green", sf::Color::Green),
	CharacterColour("Blue", sf::Color::Blue),
	CharacterColour("Yellow", sf::Color::Yellow),
	CharacterColour("Black", sf::Color::Black),
	CharacterColour("Magenta", sf::Color::Magenta),
	CharacterColour("Orange", Colour(255, 128, 0)),
	CharacterColour("Cyan", sf::Color::Cyan),
}
);
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
	return false;
}
bool ABCharacter::ExecuteRPC(uint16& id, ByteBuffer& params)
{
	RPC_EXEC_HEADER(id, params);
	RPC_EXEC_OneParam(PlaceBomb, ivec2);
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
			// Asset gets registerd at path (Where path is xyz.png) xyz.png.<colour name>
			for (const CharacterColour& colour : s_supportedColours)
			{
				// Get texture in desired colour
				sf::Texture* texture = new sf::Texture;
				texture->setSmooth(false);
				texture->setRepeated(false);

				sf::Image image;
				image.loadFromFile(defaultPath);
				CastColourFromCommonGrey(image, colour.colour);
				texture->loadFromImage(image);

				assets->RegisterTexture(defaultPath + "." + colour.name, texture);
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
		for (const CharacterColour& colour : s_supportedColours)
		{
			AnimationSheet* colAnim = new AnimationSheet;
			colAnim->SetFrameDuration(0.15f);
			colAnim->AddFrame(game->GetAssetController()->GetTexture("Resources\\Character\\" + dir + "_0.png." + colour.name));
			colAnim->AddFrame(game->GetAssetController()->GetTexture("Resources\\Character\\" + dir + "_1.png." + colour.name));
			colAnim->AddFrame(game->GetAssetController()->GetTexture("Resources\\Character\\" + dir + "_0.png." + colour.name));
			colAnim->AddFrame(game->GetAssetController()->GetTexture("Resources\\Character\\" + dir + "_2.png." + colour.name));
			assets->RegisterAnimation("Resources\\Character\\" + dir + ".anim." + colour.name, colAnim);
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
	// Load default animations
	//m_animUp = GetGame()->GetAssetController()->GetAnimation("Resources\\Character\\Up.anim");
	//m_animDown = GetGame()->GetAssetController()->GetAnimation("Resources\\Character\\Down.anim");
	//m_animLeft = GetGame()->GetAssetController()->GetAnimation("Resources\\Character\\Left.anim");
	//m_animRight = GetGame()->GetAssetController()->GetAnimation("Resources\\Character\\Right.anim");

	if (IsNetOwner())
	{
		// Load default animations
		m_animUp = GetGame()->GetAssetController()->GetAnimation("Resources\\Character\\Up.anim.green");
		m_animDown = GetGame()->GetAssetController()->GetAnimation("Resources\\Character\\Down.anim.green");
		m_animLeft = GetGame()->GetAssetController()->GetAnimation("Resources\\Character\\Left.anim.green");
		m_animRight = GetGame()->GetAssetController()->GetAnimation("Resources\\Character\\Right.anim.green");
	}
	else
	{
		m_animUp = GetGame()->GetAssetController()->GetAnimation("Resources\\Character\\Up.anim.red");
		m_animDown = GetGame()->GetAssetController()->GetAnimation("Resources\\Character\\Down.anim.red");
		m_animLeft = GetGame()->GetAssetController()->GetAnimation("Resources\\Character\\Left.anim.red");
		m_animRight = GetGame()->GetAssetController()->GetAnimation("Resources\\Character\\Right.anim.red");
	}
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

		if (m_bombKey.IsPressed())
			CallRPC_OneParam(this, PlaceBomb, GetClosestTileLocation());
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
	if (bomb != nullptr)
		bomb->AttemptToPlace(tile);
}