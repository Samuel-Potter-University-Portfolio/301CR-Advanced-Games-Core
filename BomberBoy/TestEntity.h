#pragma once
#include "Core\Entity.h"


class TestEntity : public Entity
{
public:
	TestEntity();

#ifdef BUILD_CLIENT
	/**
	* Called when this entity should be drawn to the screen
	* @param window			The window to draw to
	* @param deltaTime		Time since last draw in seconds)
	*/
	virtual void Draw(sf::RenderWindow* window, const float& deltaTime);
#endif
};

