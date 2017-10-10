#include "Includes\Core\Engine.h"
#include "Includes\Core\Game.h"


EngineInfo::EngineInfo(std::vector<string>& args) 
{
	// TODO - Parse args
}


Engine::Engine(EngineInfo* info) : m_initInfo(*info)
{
	LOG("Engine Initializing");
}

Engine::~Engine()
{
	LOG("Engine destroyed");
}

void Engine::Launch(Game* game)
{
	LOG("Launching game engine with game '%s'", game->GetName().c_str());

	// Setup game
	m_game = game;
	m_game->HookEngine(this);

	
	// Launch main loop for handling visuals
#ifdef BUILD_CLIENT
	sf::Thread m_displayThread(&Engine::DisplayLoop, this);
	m_displayThread.launch();
#endif


	// Launch into main loop
	MainLoop();


	// Wait for display thread to close
#ifdef BUILD_CLIENT
	m_displayThread.wait();
#endif

	m_game = nullptr;
	LOG("Main engine loop closed");
}
	

void Engine::MainLoop()
{
	LOG("Main game loop started");
	bUpdateMain = true;
	sf::Clock clock;


	// Launch main loop
	while (bUpdateMain)
	{
		// Tick logic 
		const float deltaTime = (float)(clock.restart().asMicroseconds()) / 1000000.0f;
		m_game->MainUpdate(deltaTime);
		// TODO

		sf::sleep(sf::milliseconds(1));
	}


	// Make sure display loop closes too
	bUpdateDisplay = false;
	LOG("Main game loop closed");
}


#ifdef BUILD_CLIENT
void Engine::DisplayLoop()
{
	LOG("Display game loop started");
	bUpdateDisplay = true;
	sf::Clock clock;


	// Open Window
	m_renderWindow = new sf::RenderWindow(sf::VideoMode(m_initInfo.windowWidth, m_initInfo.windowHeight), m_game->GetName());
	LOG("Opening window (%s, %ix%i)", m_game->GetName().c_str(), m_initInfo.windowWidth, m_initInfo.windowHeight);


	// Launch into main loop
	while (bUpdateDisplay && m_renderWindow->isOpen())
	{
		// Poll any pending events
		sf::Event event;
		while (m_renderWindow->pollEvent(event))
			HandleDisplayEvent(event);



		// Clear window
		m_renderWindow->clear();

		// Tick rendering 
		const float deltaTime = (float)(clock.restart().asMicroseconds()) / 1000000.0f;
		m_game->DisplayUpdate(deltaTime);

		// Update display (Performs any syncing aswell)
		m_renderWindow->display();
	}


	// Close window
	m_renderWindow->close();
	delete m_renderWindow;
	m_renderWindow = nullptr;


	// Make sure main loop closes too
	bUpdateMain = false;
	LOG("Display game loop closed");
}


void Engine::HandleDisplayEvent(sf::Event& event) 
{
	switch (event.type)
	{
	case sf::Event::Closed:
		Close();
		break;
	}
}

#endif