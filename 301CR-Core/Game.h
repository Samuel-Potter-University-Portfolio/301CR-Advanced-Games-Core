#pragma once
#include "Types.h"
#include "Logger.h"

#include <vector>


class Engine;


struct GameInfo
{
	string name = "Untitled";

	GameInfo(std::vector<string>& args);
};



class Game
{
private:
	string m_name;
	Engine* m_engine;

public:
	Game(GameInfo* info);
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
	* Getters & Setters
	*/
public:
	inline string GetName() { return m_name; }
};

