#pragma once
#include "ManagedClass.h"
#include "Actor.h"
#include <vector>
#include <unordered_map>


class Game;


/**
* Represents a collection of actors and provides an way to receive and propagate engine callbacks
* All children of this class should append an L to the beginning of their name
*/
class CORE_API LLevel : public ManagedObject
{
	CLASS_BODY()
	friend class NetSession;
private:
	static uint32 s_instanceCounter;
	uint32 m_instanceId;
	Game* m_game = nullptr;

	bool bIsBuilding = false;
	bool bIsDestroying = false;

	std::vector<AActor*> m_activeActors;
	std::unordered_map<uint16, AActor*> m_netActorLookup;

public:
	LLevel();
	~LLevel();


	/**
	* Callback for when the level becomes active
	* @param game		The game that the level has been registered under
	*/
	void OnLevelActive(Game* game);

	/**
	* Callback from engine for every tick by main loop
	* @param deltaTime		Time since last update (In seconds)
	*/
	void MainUpdate(const float& deltaTime);

#ifdef BUILD_CLIENT
	/**
	* Callback from engine for every tick by display
	* @param window			The window to draw to
	* @param deltaTime		Time since last update (In seconds)
	*/
	void DisplayUpdate(sf::RenderWindow* window, const float& deltaTime);
#endif

	/** Builds the level by adding any actors */
	void Build();
	/** Cleans up the level */
	void Destroy();

	/**
	* Add an actor to the level (Forfeits memory rights to level)
	* @param actor		The actor to add
	*/
	void AddActor(AActor* actor);

	/**
	* Spawns an actor into the level of the given type
	* @param actorClass			The class of the actor to spawn
	* @param location			Location to spawn the actor at
	* @param owner				The object who is seen as this object's owner
	* @returns New actor object or nullptr, if invalid
	*/
	AActor* SpawnActor(const SubClassOf<AActor>& actorClass, const vec2& location = vec2(0, 0), const OObject* owner = nullptr);
	/**
	* Spawns an actor into the level of the given type
	* @param actorClass			The class of the actor to spawn
	* @param location			Location to spawn the actor at
	* @param owner				The object who is seen as this object's owner
	* @returns New actor object or nullptr, if invalid
	*/
	template<class ActorType>
	ActorType* SpawnActor(const vec2& location = vec2(0, 0), const OObject* owner = nullptr) { return static_cast<ActorType*>(SpawnActor(ActorType::StaticClass(), location, owner)); }

protected:
	/**
	* Callback for when this level is being built
	* Actors should be spawned here
	*/
	virtual void OnBuildLevel() {}
	/**
	* Callback for when this level is being destroyed
	* Actors will automatically be destroyed, but additional cleanup should occur here
	*/
	virtual void OnDestroyLevel() {}


	/**
	* Getters & Setters
	*/
public:
	/** A unique id applied to each instance of a level */
	inline const uint32& GetInstanceID() const { return m_instanceId; }

	inline Game* GetGame() const { return m_game; }


	/** 
	* Returns all active actors 
	*/
	inline const std::vector<AActor*>& GetActiveActors() const { return m_activeActors; }

	/**
	* Return all active actors of this class
	* @param type			The class type to query for
	*/
	inline const std::vector<AActor*> GetActiveActors(const MClass* type) const 
	{
		std::vector<AActor*> output;
		for (uint32 i = 0; i < m_activeActors.size(); ++i)
			if (m_activeActors[i]->GetClass()->IsChildOf(type))
				output.emplace_back(m_activeActors[i]);
		return output;
	}

	/**
	* Return all active actors of this class type
	*/
	template<class ActorType>
	inline const std::vector<ActorType*> GetActiveActors() const 
	{
		std::vector<ActorType*> output;
		for (uint32 i = 0; i < m_activeActors.size(); ++i)
		{
			ActorType* casted = dynamic_cast<ActorType*>(m_activeActors[i]);
			if (casted != nullptr)
				output.emplace_back(casted);
		}
		return output;
	}


	/**
	* Retrieve an actor from it's instance id (Not safe unless id is known to be unique)
	* @param id			Instance id of this actor
	* @returns The actor of this id or nullptr, if not found
	*/
	AActor* GetActorByInstance(const uint32& id) const;

	/**
	* Retrieve an actor from it's network id 
	* @param id			Network id of this actor
	* @returns The actor of this id or nullptr, if not found
	*/
	AActor* GetActorByNetID(const uint32& id) const;
};