#pragma once
#include "Types.h"
#include "Logger.h"

#include <SFML/Graphics.hpp>
#include <string>


struct EngineInfo
{
	string gameTitle = "Untitled";
	uint32 windowWidth = 800;
	uint32 windowHeight = 600;

	EngineInfo(int argc, char** argv);
};


/**
* Interface for handling any subsystem relating to games
*/
class Engine
{
private:
	EngineInfo m_initInfo;
	sf::RenderWindow* m_renderWindow;

	bool bUpdateMain;
	bool bUpdateDisplay;

public:
	Engine(EngineInfo* info);
	~Engine();

	/**
	* Launch the engine into it's main loops
	*/
	void Launch();

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
};

