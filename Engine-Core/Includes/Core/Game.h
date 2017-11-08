#pragma once
#include "Common.h"
#include "Version.h"

#include "NetSerializableBase.h"
#include "ClassFactory.h"
#include "Entity.h"

#include <vector>
#include <unordered_map>


class Engine;
class Level;

/**
* Contains any relevent information about a given game
* e.g. Assets to load, Supported entity types, Supported levels etc.
*/
class CORE_API Game : public NetSerializableBase
{
private:
	string m_name;
	Engine* m_engine = nullptr;
	Version m_version;


	/** Map of all supported levels that can be loaded */
	std::unordered_map<string, Level*> m_levels;
	std::vector<Level*> m_levelLookup;

	/** Map using class hashes to link to factories	*/
	std::unordered_map<uint32, ClassFactory<Entity>*> m_entityTypes;
	std::vector<ClassFactory<Entity>*> m_entityTypeLookup;

	string defaultLevel = "Main";
	Level* currentLevel = nullptr;


public:
	Game(string name, Version version);
	~Game();

	/**
	* Callback for when the engine prepares to launch the game
	* @param engine		Engine in question
	*/
	void HookEngine(Engine* engine);

	/**
	* Callback from engine for every tick by main
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
	* Registers a given level, so it may be loaded by the game
	* (Memory management goes to the game)
	* @param level		The level in question
	*/
	void RegisterLevel(Level* level);

	/**
	* Switch to level with this registered name
	* @param levelName		The name of the level
	* @returns If switch was successful
	*/
	bool SwitchLevel(string levelName);

	/**
	* Registers a given entity type, so that it may be spawned later into the level
	* (Memory management goes to the game)
	* @param entityType		Factory to create the given entity
	*/
	void RegisterEntity(ClassFactory<Entity>* entityType);


	/**
	* Fetch the factory for a given entity from its name
	* @param id			The id that the type is registered with
	* @return The factory that can be used to build the entity
	*/
	ClassFactory<Entity>* GetEntityFactoryFromID(uint32 id);

	/**
	* Fetch the factory for a given entity from its name
	* @param hash		The hash of the entity type
	* @return The factory that can be used to build the entity
	*/
	ClassFactory<Entity>* GetEntityFactoryFromHash(uint32 hash);


public:
	/**
	* Encode all required information about this object ready to send over net
	* @param buffer			The buffer to fill with all this information
	* @param socketType		The socket type this will be sent over
	*/
	virtual void PerformNetEncode(ByteBuffer& buffer, const SocketType& socketType) override;

	/**
	* Decode any information that has reached this object
	* @param buffer			The buffer to fill with all this information
	* @param socketType		The socket type this was recieved by
	*/
	virtual void PerformNetDecode(ByteBuffer& buffer, const SocketType& socketType) override;


	/**
	* Getters & Setters
	*/
public:
	inline Engine* GetEngine() const  { return m_engine; }
	inline Level* GetCurrentLevel() const { return currentLevel; }

	inline string GetName() const { return m_name; }
	inline const Version& GetVersionNo() const { return m_version; }

	inline string GetDefaultLevelName() const { return defaultLevel; }
	inline void SetDefaultLevelName(string levelName) { defaultLevel = levelName; }
};