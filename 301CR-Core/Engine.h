#pragma once
#include "Types.h"
#include "Logger.h"

#include <vector>
#include <string>
#include <SFML/Graphics.hpp>


class Game;


struct EngineInfo
{
	uint32 windowWidth = 800;
	uint32 windowHeight = 600;

	EngineInfo(std::vector<string>& args);
};


/**
* Interface for handling any subsystem relating to games
*/
class Engine
{
private:
	EngineInfo m_initInfo;
	Game* m_game;

	bool bUpdateMain;
	bool bUpdateDisplay;

#ifdef BUILD_CLIENT
	sf::RenderWindow* m_renderWindow;
#endif

public:
	Engine(EngineInfo* info);
	~Engine();

	/**
	* Launch the engine into it's main loops
	* @param game		The game for the engine to run
	*/
	void Launch(Game* game);

	/**
	* Flags the engine to close itself
	*/
	inline void Close() { bUpdateMain = false; bUpdateDisplay = false; }

private:
	/**
	* The engine's main logic loop
	*/
	void MainLoop();

#ifdef BUILD_CLIENT
	/**
	* The engine's main display loop
	*/
	void DisplayLoop();

	/**
	* Callback for when SFML event occurs
	* @param event		The event information in question
	*/
	void HandleDisplayEvent(sf::Event& event);
#endif

	/**
	* Getters and setters
	*/
public:

#ifdef BUILD_CLIENT
	inline sf::RenderWindow* GetDisplayWindow() { return m_renderWindow; }
#endif
};

