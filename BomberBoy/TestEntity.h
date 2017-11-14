#pragma once
#include "Core\Actor.h"


class ATestEntity : public AActor
{
	CLASS_BODY()
public:
	ATestEntity();


protected:
	virtual bool FetchRPCIndex(const char* funcName, uint16& outID) const;
	virtual bool ExecuteRPC(uint16& id, ByteBuffer& params);

protected:
	/**
	* Callback for entity's level first tick
	*/
	virtual void OnBegin();

	virtual void OnDestroy();

	/**
	* Callback for level first tick
	* @param deltaTime		Time since last tick in seconds
	*/
	virtual void OnTick(const float& deltaTime);

	sf::Vector2f startPos;
	float timer;
	void ResetPosition();
	void MoveTo(float x, float y);
	void PrintTime(float time, float time2);

#ifdef BUILD_CLIENT
	/**
	* Called when this entity should be drawn to the screen
	* @param window			The window to draw to
	* @param deltaTime		Time since last draw in seconds)
	*/
	virtual void OnDraw(sf::RenderWindow* window, const float& deltaTime) override;
#endif
};

