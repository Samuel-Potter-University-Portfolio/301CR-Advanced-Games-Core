#pragma once
#include "Common.h"
#include "Version.h"
#include "NetController.h"
#include "NetSession.h"
#include "InputController.h"

#include <vector>
#include <string>
#include <SFML/Graphics.hpp>


class Game;


/**
* Main controller and central controller for any subsystems
*/
class CORE_API Engine
{
private:
	Version m_version;
	NetIdentity m_defaultNetIdentity;

	Game* m_game = nullptr;
	NetController* m_netController = nullptr;
	InputController* m_inputController = nullptr;

	uvec2 m_desiredResolution;
	bool bUpdateMain;
	bool bUpdateDisplay;

	uint32 m_mainTickRate = 50;
	uint32 m_mainSleepRate = 1000 / m_mainTickRate;

#ifdef BUILD_CLIENT
	sf::RenderWindow* m_renderWindow = nullptr;
#endif

public:
	Engine(std::vector<string>& args);
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
	inline sf::RenderWindow* GetDisplayWindow() const { return m_renderWindow; }
#endif
	inline Game* GetGame() const { return m_game; }
	inline const Version& GetVersionNo() const { return m_version; }

	inline NetController* GetNetController() { return m_netController; }

	inline const NetIdentity& GetDefaultNetIdentity() const { return m_defaultNetIdentity; }

	inline const uint32& GetMainTickRate() const { return m_mainTickRate; }
	inline void SetMainTickRate(const uint32& v) { m_mainTickRate = (v == 0 ? 1 : v); m_mainSleepRate = 1000 / m_mainTickRate; }
};
