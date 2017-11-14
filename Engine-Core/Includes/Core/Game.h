#pragma once
#include "Common.h"
#include "Version.h"

#include "Level.h"
#include "Actor.h"
#include "Object.h"

#include <vector>
#include <unordered_map>


class Engine;


/**
* Contains any relevent information about a given game
* e.g. Assets to load, Supported actor types, Supported levels etc.
*/
class CORE_API Game
{
private:
	string m_name;
	Engine* m_engine = nullptr;
	Version m_version;

	SubClassOf<LLevel> m_defaultLevel;
	SubClassOf<LLevel> m_defaultNetLevel;
	LLevel* m_currentLevel = nullptr;

	std::unordered_map<uint16, SubClassOf<LLevel>> m_registeredLevels;
	std::unordered_map<uint16, SubClassOf<OObject>> m_registeredObjectTypes;
	std::unordered_map<uint16, SubClassOf<AActor>> m_registeredActorTypes;

	std::vector<OObject*> m_activeObjects;
	std::unordered_map<uint16, OObject*> m_netObjectLookup;

public:
	Game(string name, Version version);
	~Game();

	/**
	* Callback for when the engine prepares to launch the game
	* @param engine		Engine in question
	*/
	void OnGameHooked(Engine* engine);

	/**
	* Callback from engine for every tick by main loop
	* @param deltaTime		Time since last update (In seconds)
	*/
	void MainUpdate(const float& deltaTime);

#ifdef BUILD_CLIENT
	/**
	* Callback from engine for every tick by display
	* @param deltaTime		Time since last update (In seconds)
	*/
	void DisplayUpdate(const float& deltaTime);
#endif

	/**
	* Registers a given class, so it may be loaded by the game
	* Usage should be with subclasses of Objects, Actors or Levels
	* @param classType			The class of the type to register
	*/
	void RegisterClass(const MClass* classType);

	/**
	* Is there a registered object with this id
	* @param id				The id to lookup
	* @returns If the game has a registered object type using that id
	*/
	inline bool IsRegisteredObject(const uint16& id) const { return m_registeredObjectTypes.find(id) != m_registeredObjectTypes.end(); }
	/**
	* Is there a registered actor with this id
	* @param id				The id to lookup
	* @returns If the game has a registered actor type using that id
	*/
	inline bool IsRegisteredActor(const uint16& id) const { return m_registeredActorTypes.find(id) != m_registeredActorTypes.end(); }
	/**
	* Is there a registered level with this id
	* @param id				The id to lookup
	* @returns If the game has a registered level using that id
	*/
	inline bool IsRegisteredLevel(const uint16& id) const { return m_registeredLevels.find(id) != m_registeredLevels.end(); }


	/**
	* Switch level to this class (If registered)
	* @param levelType			The class of the level to switch to
	* @returns True if the level switches
	*/
	bool SwitchLevel(const SubClassOf<LLevel>& levelType);
	/**
	* Switch level to this id (If registered)
	* @param levelId			The class of the level to switch to
	* @returns True if the level switches
	*/
	bool SwitchLevel(const uint16& levelId);


	/**
	* Add an object to the game (Forfeits memory rights to level)
	* @param object		The object to add
	*/
	void AddObject(OObject* object);

	/**
	* Spawns an object of the given type
	* @param objectClass		The class of the object to spawn
	* @returns New object or nullptr, if invalid
	*/
	OObject* SpawnObject(const SubClassOf<OObject>& objectClass);
	/**
	* Spawns an object of the given type
	* @param objectClass		The class of the object to spawn
	* @returns New actor object or nullptr, if invalid
	*/
	template<class ObjectType>
	ObjectType* SpawnObject(const SubClassOf<ObjectType>& objectClass = ObjectType::StaticClass()) { return static_cast<ObjectType*>(SpawnObject(objectClass)) }


	/**
	* Getters & Setters
	*/
public:
	inline Engine* GetEngine() const  { return m_engine; }
	inline LLevel* GetCurrentLevel() const { return m_currentLevel; }

	inline string GetName() const { return m_name; }
	inline const Version& GetVersionNo() const { return m_version; }

	inline const SubClassOf<LLevel>& GetDefaultLevel() const { return m_defaultLevel; }
	inline void SetDefaultLevel(const SubClassOf<LLevel>& level) { m_defaultLevel = level; }
	inline const SubClassOf<LLevel>& GetDefaultNetLevel() const { return m_defaultNetLevel; }
	inline void SetDefaultNetLevel(const SubClassOf<LLevel>& level) { m_defaultNetLevel = level; }

	NetSession* GetSession() const;

	/**
	* Returns all active objects
	*/
	inline const std::vector<OObject*>& GetActiveObjects() const { return m_activeObjects; }

	/**
	* Return all active objects of this class
	* @param type			The class type to query for
	*/
	inline const std::vector<OObject*> GetActiveObjects(const MClass* type) const
	{
		std::vector<OObject*> output;
		for (uint32 i = 0; i < m_activeObjects.size(); ++i)
			if (m_activeObjects[i]->GetClass()->IsChildOf(type))
				output.emplace_back(m_activeObjects[i]);
		return output;
	}

	/**
	* Return all active objects of this class type
	*/
	template<class ObjectType>
	inline const std::vector<ObjectType*> GetActiveObjects() const
	{
		std::vector<ObjectType*> output;
		for (uint32 i = 0; i < m_activeObjects.size(); ++i)
		{
			ObjectType* casted = dynamic_cast<ObjectType*>(m_activeObjects[i]);
			if (casted != nullptr)
				output.emplace_back(casted);
		}
		return output;
	}
};