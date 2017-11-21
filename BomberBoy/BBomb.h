#pragma once
#include "BTileableActor.h"


class ABBomb : public ABTileableActor
{
	CLASS_BODY()
private:
	friend class ABCharacter;
	ABCharacter* m_parent = nullptr;

	const vec2 m_drawSize;
	const vec2 m_drawOffset;
	const AnimationSheet* m_animation = nullptr;

public:
	ABBomb();

	virtual void OnBegin() override;
	//virtual void OnDestroy() override;

	/**
	* Registers the needed assets for ABBomb
	* @param game			Where to register the assets
	*/
	static void RegisterAssets(Game* game);


	virtual void OnTick(const float& deltaTime) override;
	//virtual void OnTick(const float& deltaTime) override;
#ifdef BUILD_CLIENT
	virtual void OnDraw(sf::RenderWindow* window, const float& deltaTime) override;
#endif

	/**
	* Attempt to place this bomb at this location (tile)
	* @param location		The location of the tile to place this bomb at
	* @returns If successful
	*/
	bool AttemptToPlace(const ivec2& location);
};

