#include "Game.h"
#include "Level.h"


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
}


void Game::MainUpdate(const float& deltaTime) 
{
}

#ifdef BUILD_CLIENT
void Game::DisplayUpdate(const float& deltaTime) 
{
}
#endif


void Game::RegisterLevel(Level* level) 
{
	m_levels.push_back(level);
	level->HookGame(this);
}

bool Game::SwitchLevel(string levelName)
{
	// Attempt to load level with given name
	for(Level* level : m_levels)
		if (level->GetName() == levelName)
		{
			// Close current level
			if (currentLevel != nullptr)
			{
				LOG("Closing level '%s'", currentLevel->GetName().c_str());
				currentLevel->DestroyLevel();
				currentLevel = nullptr;
			}

			// Load new level
			LOG("Loading level '%s'", levelName.c_str());
			currentLevel = level;
			currentLevel->BuildLevel();
			return true;
		}


	LOG_ERROR("Unable to load level '%s'", levelName.c_str());
	return false;
}