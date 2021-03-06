#pragma once
#include "ManagedClass.h"
#include "Actor.h"
#include <vector>
#include <unordered_map>

#include "LevelController.h"
#include "HUD.h"


class Game;


/**
* Represents a collection of actors and provides an way to receive and propagate engine callbacks
* All children of this class should append an L to the beginning of their name
*/
class CORE_API LLevel : public ManagedObject
{
	CLASS_BODY()
	friend class NetSession;
	friend class NetHostSession;
	friend class NetRemoteSession;
private:
	static uint16 s_instanceCounter;
	uint16 m_instanceId;
	Game* m_game = nullptr;

	bool bIsBuilding = false;
	bool bIsDestroying = false;

	std::vector<AActor*> m_activeActors;
	std::unordered_map<uint16, AActor*> m_netActorLookup;

	std::vector<AActor*> m_drawnActors;

protected:
	/// Class type to use for the level controller
	SubClassOf<ALevelController> levelControllerClass;
	ALevelController* m_levelController = nullptr;

	/// Class type to use for the HUD
	SubClassOf<AHUD> hudClass;
	AHUD* m_hud = nullptr;

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
	template<class ActorType>
	ActorType* SpawnActor(const SubClassOf<AActor>& actorClass = ActorType::StaticClass(), const OObject* owner = nullptr) { return static_cast<ActorType*>(SpawnActor<AActor>(*actorClass, owner)); }
	/**
	* Spawns an actor into the level of the given type
	* @param actorClass			The class of the actor to spawn
	* @param location			Location to spawn the actor at
	* @param owner				The object who is seen as this object's owner
	* @returns New actor object or nullptr, if invalid
	*/
	template<>
	AActor* SpawnActor(const SubClassOf<AActor>& actorClass, const OObject* owner);

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
	inline const uint16& GetInstanceID() const { return m_instanceId; }

	inline Game* GetGame() const { return m_game; }
	inline ALevelController* GetLevelController() const { return m_levelController; }
	inline AHUD* GetHUD() const { return m_hud; }


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
			if (m_activeActors[i]->GetClass()->IsChildOf(type) && !m_activeActors[i]->IsDestroyed())
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
			if (casted != nullptr && !casted->IsDestroyed())
				output.emplace_back(casted);
		}
		return output;
	}

	/**
	* Get the first actor of this type
	* @param onlyOwned		Ignore objects that aren't owned by this client
	*/
	template<class ActorType>
	inline ActorType* GetFirstActor(const bool& onlyOwned = false) const
	{
		for (uint32 i = 0; i < m_activeActors.size(); ++i)
		{
			ActorType* casted = dynamic_cast<ActorType*>(m_activeActors[i]);
			if (casted != nullptr && (!onlyOwned || casted->IsNetOwner()) && !casted->IsDestroyed())
				return casted;
		}
		return nullptr;
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