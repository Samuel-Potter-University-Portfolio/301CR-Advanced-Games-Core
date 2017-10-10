#pragma once
#include "Common.h"
#include "Game.h"
#include "Engine.h"


class Entity;


/**
* Represents a collections of entities which all form a level
*/
class CORE_API Level
{
private:
	string m_name;
	Game* m_game;

	std::vector<Entity*> m_entities;
	class Camera* m_camera;

public:
	Level(string name);
	~Level();

	/**
	* Callback for when the game registers this level
	* @param game		Game in question
	*/
	void HookGame(Game* game);

	/**
	* Called when this level comes into usage
	*/
	virtual void BuildLevel() = 0;
	/**
	* Called when this level is about to go out of usage
	*/
	virtual void DestroyLevel();

	/**
	* Called when an entity should be added to a level
	* @param entity			Desired entity
	*/
	void AddEntity(Entity* entity);

	/**
	* Spawn an entity into this level from it's name
	* @param name			The name that the entity was registered under
	* @returns The new entity that has spawned into the world
	*/
	Entity* SpawnEntity(string name);

	/**
	* Getters and setters
	*/
public:
	inline string GetName() const { return m_name; }
	inline Game* GetGame() const { return m_game; }

	inline std::vector<Entity*>& GetEntities() { return m_entities; }
	inline std::vector<Entity*> GetEntities() const { return m_entities; }
	inline Camera* GetMainCamera() const { return m_camera; }
};