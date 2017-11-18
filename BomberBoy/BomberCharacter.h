#pragma once
#include "Core\Core-Common.h"


/**
* Represents a colour that players can be
*/
struct CharacterColour 
{
	const string	name;
	const Colour	colour;

	CharacterColour(const string& name, const Colour& colour)
		: name(name), colour(colour) {}
};


/**
* Represents player characters used in BomberBoy
*/
class ABomberCharacter : public AActor
{
	CLASS_BODY()
public:
	static const std::vector<CharacterColour> s_supportedColours;

private:
	const AnimationSheet* m_animUp = nullptr;
	const AnimationSheet* m_animDown = nullptr;
	const AnimationSheet* m_animLeft = nullptr;
	const AnimationSheet* m_animRight = nullptr;

	const vec2 m_drawSize;
	const vec2 m_drawOffset;

public:
	ABomberCharacter();

	virtual void OnBegin() override;
	//virtual void OnDestroy() override;

	/**
	* Registers the needed assets for ABomberCharacter
	* @param game			Where to register the assets
	*/
	static void RegisterAssets(Game* game);


	virtual void OnTick(const float& deltaTime) override;
#ifdef BUILD_CLIENT
	virtual void OnDraw(sf::RenderWindow* window, const float& deltaTime) override;
#endif
};

