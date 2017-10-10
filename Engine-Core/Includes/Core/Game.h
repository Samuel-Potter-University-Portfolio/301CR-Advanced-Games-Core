#pragma once
#include "Common.h"
#include "Types.h"
#include "Logger.h"

#include <vector>


class Engine;
class Level;

/**
* Contains any relevent information about a given game
* e.g. Assets to load, Supported entity types, Supported levels etc.
*/
class CORE_API Game
{
private:
	string m_name;
	Engine* m_engine = nullptr;

	std::vector<Level*> m_levels;
	string defaultLevel = "Main";
	Level* currentLevel = nullptr;

public:
	Game(string name);
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
	* Getters & Setters
	*/
public:
	inline Engine* GetEngine() { return m_engine; m_engine; }

	inline string GetName() const { return m_name; }

	inline string GetDefaultLevelName() const { return defaultLevel; }
	inline void SetDefaultLevelName(string levelName) { defaultLevel = levelName; }
};