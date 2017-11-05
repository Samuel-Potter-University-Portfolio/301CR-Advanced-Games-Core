#include "Includes\Core\Game.h"
#include "Includes\Core\Level.h"
#include "Includes\Core\Entity.h"


Game::Game(string name, Version version) :
	m_name(name),
	m_version(version)
{
	// Reserve 0 ids as null
	m_entityTypeLookup.push_back(nullptr);
	m_levelLookup.push_back(nullptr);
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
	m_levelLookup.push_back(level);
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

	// Recentre camera
#ifdef BUILD_CLIENT
	sf::RenderWindow* window = m_engine->GetDisplayWindow();
	if(window != nullptr)
		window->setView(window->getDefaultView());
#endif

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
	if (m_entityTypes.find(entityType->GetHash()) != m_entityTypes.end())
	{
		LOG_ERROR("Cannot register multiple entities with name '%s'", entityType->GetName());
	}

	// Give type a unique id
	m_entityTypes[entityType->GetHash()] = entityType;
	m_entityTypeLookup.push_back(entityType);
	entityType->SetID(m_entityTypeLookup.size() - 1);
}

ClassFactory<Entity>* Game::GetEntityFactoryFromID(uint32 id) 
{
	if (id >= m_entityTypeLookup.size())
	{
		LOG_ERROR("Cannot find registered entity of type id '%i'", id);
		return nullptr;
	}
	return m_entityTypeLookup[id];
}

ClassFactory<Entity>* Game::GetEntityFactoryFromHash(uint32 hash)
{
	auto it = m_entityTypes.find(hash);
	if (it == m_entityTypes.end())
	{
		LOG_ERROR("Cannot find registered entity of hash '%i'", hash);
		return nullptr;
	}

	return it->second;
}