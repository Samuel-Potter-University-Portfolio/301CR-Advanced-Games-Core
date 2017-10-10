#include "Includes\Core\Game.h"
#include "Includes\Core\Level.h"
#include "Includes\Core\Entity.h"


Game::Game(string name)
{
	m_name = name;
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

	// Delete all entities
	for (auto& pair : m_entityTypes)
		delete pair.second;

	// Delete all levels
	for (auto& pair : m_levels)
		delete pair.second;

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
	if (currentLevel == nullptr)
		return;

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
	if (m_levels.find(level->GetName()) != m_levels.end())
	{
		LOG_ERROR("Cannot register multiple levels with name '%s'", level->GetName().c_str());
	}

	m_levels[level->GetName()] = level;
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
	Level* level = m_levels[levelName];

	// Load new level
	if (level != nullptr) 
	{
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

void Game::RegisterEntity(ClassFactory<Entity>* entityType) 
{
	if (m_entityTypes.find(entityType->GetName()) != m_entityTypes.end())
	{
		LOG_ERROR("Cannot register multiple entities with name '%s'", entityType->GetName());
	}

	m_entityTypes[entityType->GetName()] = entityType;
}