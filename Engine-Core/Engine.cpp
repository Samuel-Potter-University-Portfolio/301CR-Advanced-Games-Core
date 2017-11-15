#include "Includes\Core\Engine.h"
#include "Includes\Core\Game.h"



Engine::Engine(std::vector<string>& args) :
	m_version(0,1,1)
{
	LOG("Engine Initializing");
	LOG("\t-Engine Version (%i.%i.%i)", m_version.major, m_version.minor, m_version.patch);

	m_netController = new NetController(this);
	m_desiredResolution = uvec2(800, 600);
	m_defaultNetIdentity.ip = sf::IpAddress::LocalHost;
	m_defaultNetIdentity.port = 20010;
}

Engine::~Engine()
{
	if (m_netController != nullptr)
		delete m_netController;

	LOG("Engine destroyed");
}

void Engine::Launch(Game* game)
{
	LOG("Launching game engine with game '%s'", game->GetName().c_str());
	LOG("\t-Game Version (%i.%i.%i)", game->GetVersionNo().major, game->GetVersionNo().minor, game->GetVersionNo().patch);

	// Setup game
	m_game = game;
	m_game->OnGameHooked(this);

	
#ifdef BUILD_CLIENT
	// Launch display loop for handling visuals
	sf::Thread m_displayThread(&Engine::DisplayLoop, this);
	m_displayThread.launch();

#else
	// Automatically open session on server
	if (!m_netController->HostSession(m_defaultNetIdentity))
	{
		LOG_ERROR("Aborting launch (Failed to launch session)");
		return;
	}

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