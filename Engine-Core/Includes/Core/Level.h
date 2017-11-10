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
	friend class NetSession;
	string m_name;
	Game* m_game;
	uint32 m_id;

	std::vector<Entity*> m_entities;
	std::map<uint16, Entity*> m_netEntities;
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
	* Callback for after the level is successfully built and loaded
	*/
	void OnPostLoad();

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
	Type* SpawnEntity(uint16 ownerId = 0)
	{
		ClassFactory<Entity>* factory = m_game->GetEntityFactoryFromHash(typeid(Type).hash_code());
		if (factory != nullptr)
		{
			Type* e = factory->New<Type>();
			e->m_typeId = factory->GetID();
			e->m_networkOwnerId = ownerId;
			AddEntity(e);
			return e;
		}
		else
			return nullptr;
	}

	/**
	* Retrieve an entity from it's network id
	* @param netId			The entity's unique network id
	* @returns Entity with id or null, if not found
	*/
	Entity* GetEntityFromNetId(const uint16& netId) const;

	/**
	* Retrieve an entity from it's instance id
	* @param id				The entity's unique instance id
	* @returns Entity with id or null, if not found
	*/
	Entity* GetEntityFromInstanceId(const uint16& id) const;

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