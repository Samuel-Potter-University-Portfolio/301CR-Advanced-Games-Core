#include "Includes\Core\Engine.h"
#include "Includes\Core\Game.h"



Engine::Engine(std::vector<string>& args) :
	m_version(0,1,2)
{
	LOG("Engine Initializing");
	LOG("\t-Engine Version (%i.%i.%i)", m_version.major, m_version.minor, m_version.patch);

#ifdef BUILD_CLIENT
	m_inputController = new InputController;
#else
	m_inputController = nullptr;
#endif
	m_netController = new NetController(this);

	m_desiredResolution = uvec2(800, 600);
}

Engine::~Engine()
{
	if (m_netController != nullptr)
		delete m_netController;
	if (m_inputController != nullptr)
		delete m_inputController;

	LOG("Engine destroyed");
}

void Engine::Launch(Game* game)
{
	LOG("Launching game engine with game '%s'", game->GetName().c_str());
	LOG("\t-Game Version (%i.%i.%i)", game->GetVersionNo().major, game->GetVersionNo().minor, game->GetVersionNo().patch);
	m_game = game;

	
#ifdef BUILD_CLIENT
	// Launch display loop for handling visuals
	sf::Thread m_displayThread(&Engine::DisplayLoop, this);
	m_displayThread.launch();

#else
	// Automatically open session on server
	if (!m_netController->HostSession(m_netController->GetLocalIdentity()))
	{
		LOG_ERROR("Aborting launch (Failed to launch session)");
		return;
	}

#endif

	// Launch into main loop
	m_game->OnGameHooked(this);
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
		m_netController->HandleUpdate(deltaTime);

		// Sleep a little
		// TODO - More elegant checks to compensate for large loops
		sf::sleep(sf::milliseconds(m_mainSleepRate));
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
	sf::ContextSettings settings;
	settings.antialiasingLevel = 4;

	m_renderWindow = new sf::RenderWindow(
		sf::VideoMode(m_desiredResolution.x, m_desiredResolution.y),
		m_game->GetName(), 
		sf::Style::Default, 
		settings
	);
	m_renderWindow->setFramerateLimit(200);

	LOG("Opening window (%s, %ix%i)", m_game->GetName().c_str(), m_desiredResolution.x, m_desiredResolution.y);


	// Launch into main loop
	while (bUpdateDisplay && m_renderWindow->isOpen())
	{
		// Poll any pending events
		m_inputController->PrePoll(m_game);
		sf::Event event;
		while (m_renderWindow->pollEvent(event))
			HandleDisplayEvent(event);
		m_inputController->PostPoll(m_game);


		// Clear window
		m_renderWindow->clear(Colour(36, 37, 47, 255));


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
	if (event.type == sf::Event::Closed)
		Close();
	else
		m_inputController->UpdateEvent(event);
}

#endif