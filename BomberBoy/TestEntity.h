#pragma once
#include "Core\Entity.h"


class TestEntity : public Entity
{
public:
	TestEntity();


protected:
	virtual bool FetchRPCIndex(const char* funcName, uint16& outID) const;
	virtual bool ExecuteRPC(uint16& id, ByteBuffer& params);

protected:
	/**
	* Callback for entity's level first tick
	*/
	virtual void OnBegin();

	/**
	* Callback for level first tick
	* @param deltaTime		Time since last tick in seconds
	*/
	virtual void OnTick(const float& deltaTime);

	sf::Vector2f startPos;
	float timer;
	void ResetPosition();
	void MoveTo(float x, float y);

#ifdef BUILD_CLIENT
	/**
	* Called when this entity should be drawn to the screen
	* @param window			The window to draw to
	* @param deltaTime		Time since last draw in seconds)
	*/
	virtual void Draw(sf::RenderWindow* window, const float& deltaTime);
#endif
};

