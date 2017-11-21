#include "BCharacter.h"
#include "BLevelArena.h"


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


ABCharacter::ABCharacter() :
	m_drawSize(vec2(32.0f, 42.0f)),
	m_drawOffset(vec2(5.0f, 0.0f)),

	m_upKey(sf::Keyboard::Key::W),
	m_downKey(sf::Keyboard::Key::S),
	m_leftKey(sf::Keyboard::Key::A),
	m_rightKey(sf::Keyboard::Key::D)
{
	bIsNetSynced = true;
	bIsTickable = true;
	m_drawingLayer = 3;

	m_movementSpeed = 0.4f;

	RegisterKeybinding(&m_upKey);
	RegisterKeybinding(&m_downKey);
	RegisterKeybinding(&m_leftKey);
	RegisterKeybinding(&m_rightKey);
}


/**
* Replaces parts of a texture with a given colour
* Used to generate player/team colours
*/
static void ReplaceGreyMask(sf::Image& image, const sf::Color& colour)
{
	for (uint32 x = 0; x < image.getSize().x; ++x)
		for (uint32 y = 0; y < image.getSize().y; ++y)
		{
			sf::Color pc = image.getPixel(x, y);
			if (pc == sf::Color(170, 170, 170) || pc == sf::Color(127, 127, 127)) // Multiply these colours by the given colour
				image.setPixel(x, y, pc * colour);
		}
}
static sf::Texture* GetTextureInColour(const string& path, const sf::Color& colour)
{
	sf::Texture* texture = new sf::Texture;
	texture->setSmooth(false);
	texture->setRepeated(false);

	sf::Image image;
	image.loadFromFile(path);
	ReplaceGreyMask(image, colour);

	texture->loadFromImage(image);
	return texture;
}


void ABCharacter::RegisterAssets(Game* game) 
{
	AssetController* assets = game->GetAssetController();
	game->RegisterClass(StaticClass());


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
				assets->RegisterTexture(defaultPath + "." + colour.name, GetTextureInColour(defaultPath, colour.colour));
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