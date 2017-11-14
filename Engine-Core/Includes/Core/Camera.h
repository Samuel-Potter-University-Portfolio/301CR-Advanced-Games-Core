#pragma once
#include "Common.h"
#include "Actor.h"


class CORE_API ACamera : public AActor
{
	CLASS_BODY()
public:
	ACamera();

#ifdef BUILD_CLIENT
	/**
	* Called when this entity should be drawn to the screen
	* @param window			The window to draw to
	* @param deltaTime		Time since last draw in seconds)
	*/
	virtual void OnDraw(sf::RenderWindow* window, const float& deltaTime) override;
#endif
};