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
	uint32 m_id;

	std::vector<Entity*> m_entities;
	uint32 m_entityCounter;

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
	template<class Type>
	Type* SpawnEntity()
	{
		ClassFactory<Entity>* factory = m_game->GetEntityFactoryFromHash(typeid(Type).hash_code());
		if (factory != nullptr)
		{
			Type* e = factory->New<Type>();
			e->m_typeId = factory->GetID();
			AddEntity(e);
			return e;
		}
		else
			return nullptr;
	}

	/**
	* Getters and setters
	*/
public:
	inline string GetName() const { return m_name; }
	inline Game* GetGame() const { return m_game; }

	inline std::vector<Entity*>& GetEntities() { return m_entities; }
	inline std::vector<Entity*> GetEntities() const { return m_entities; }

	inline uint32 GetID() { return m_id; }
};