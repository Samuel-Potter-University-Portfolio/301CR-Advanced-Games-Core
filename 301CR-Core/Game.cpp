#include "Game.h"
#include "Level.h"
#include "Entity.h"


GameInfo::GameInfo(std::vector<string>& args)
{
	// TODO - Parse any command line args
}

Game::Game(GameInfo* info)
{
	m_name = info->name;
}

Game::~Game()
{
	// Close current level
	if (currentLevel != nullptr)
	{
		LOG("Closing level '%s'", currentLevel->GetName().c_str());
		currentLevel->DestroyLevel();
		currentLevel = nullptr;
	}

	// Delete all levels
	for (Level* level : m_levels)
		delete level;

	LOG("Closed game '%s'", m_name.c_str());
}


void Game::HookEngine(Engine* engine)
{
	m_engine = engine;
	SwitchLevel(defaultLevel);
}


void Game::MainUpdate(const float& deltaTime) 
{
	if (currentLevel == nullptr)
		return;

	std::vector<Entity*> entities = currentLevel->GetEntities();
	for (Entity* entity : entities)
		entity->HandleMainUpdate(deltaTime);
}

#ifdef BUILD_CLIENT
void Game::DisplayUpdate(const float& deltaTime) 
{
	std::vector<Entity*> entities = currentLevel->GetEntities();
	sf::RenderWindow* window = m_engine->GetDisplayWindow();

	for (uint8 i = 0; i < 20; ++i)
	{
		for (Entity* entity : entities)
		{
			if (entity->GetSortingLayer() == i)
				entity->Draw(window, deltaTime);
		}
	}
}
#endif


void Game::RegisterLevel(Level* level) 
{
	m_levels.push_back(level);
	level->HookGame(this);
}

bool Game::SwitchLevel(string levelName)
{
	// Close current level
	if (currentLevel != nullptr)
	{
		LOG("Closing level '%s'", currentLevel->GetName().c_str());
		currentLevel->DestroyLevel();
		currentLevel = nullptr;
	}

	// Attempt to load level with given name
	for(Level* level : m_levels)
		if (level->GetName() == levelName)
		{
			// Load new level
			LOG("Loading level '%s'", levelName.c_str());
			currentLevel = level;
			currentLevel->BuildLevel();
			return true;
		}

	
	// Attempt to load into default level
	if (levelName == defaultLevel)
	{
		LOG_ERROR("Unable to load level default level '%s'", defaultLevel.c_str());
	}
	else
	{
		LOG_ERROR("Unable to load level '%s' (Attempting to load default)", levelName.c_str());
		SwitchLevel(defaultLevel);
	}

	return false;
}