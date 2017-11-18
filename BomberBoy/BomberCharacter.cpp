#include "BomberCharacter.h"


CLASS_SOURCE(ABomberCharacter)


const std::vector<CharacterColour> ABomberCharacter::s_supportedColours(
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


ABomberCharacter::ABomberCharacter() :
	m_drawSize(vec2(16.0f, 21.0f) * 2.0f),
	m_drawOffset(vec2(0.0f, 0.0f))
{
	bIsNetSynced = true;
	bIsTickable = true;
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


void ABomberCharacter::RegisterAssets(Game* game) 
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


void ABomberCharacter::OnBegin()
{
	Super::OnBegin();

#ifdef BUILD_CLIENT
	// Load default animations
	m_animUp = GetGame()->GetAssetController()->GetAnimation("Resources\\Character\\Up.anim");
	m_animDown = GetGame()->GetAssetController()->GetAnimation("Resources\\Character\\Down.anim");
	m_animLeft = GetGame()->GetAssetController()->GetAnimation("Resources\\Character\\Left.anim");
	m_animRight = GetGame()->GetAssetController()->GetAnimation("Resources\\Character\\Right.anim");
#endif
}

void ABomberCharacter::OnTick(const float& deltaTime) 
{
#ifdef BUILD_SERVER
	Translate(vec2(0, 35) * deltaTime);
	if (GetLocation().y > 100)
		SetLocation(vec2(GetLocation().x, -100));
#endif
}

#ifdef BUILD_CLIENT
void ABomberCharacter::OnDraw(sf::RenderWindow* window, const float& deltaTime) 
{
	const AnimationSheet* anim = m_animDown;

	sf::RectangleShape rect;
	rect.setPosition(GetLocation() + m_drawOffset);
	rect.setSize(m_drawSize);
	if(anim != nullptr)
		rect.setTexture(anim->GetCurrentFrame());
	window->draw(rect);
}
#endif

