#include "Includes\Core\Game.h"
#include "Includes\Core\Engine.h"


Game::Game(string name, Version version) :
	m_name(name),
	m_version(version)
{
}

Game::~Game()
{
	// Close current level
	if (m_currentLevel != nullptr)
	{
		LOG("Closing level '%s'", m_currentLevel->GetClass()->GetName().c_str());
		m_currentLevel->Destroy();
		delete m_currentLevel;
		m_currentLevel = nullptr;
	}

	LOG("Closed game '%s'", m_name.c_str());
}


void Game::MainUpdate(const float& deltaTime)
{
	if (m_currentLevel != nullptr)
		m_currentLevel->MainUpdate(deltaTime);


	// Perform cleanup
	for (uint32 i = 0; i < m_activeObjects.size(); ++i)
	{
		OObject*& object = m_activeObjects[i];
		if (!object->IsDestroyed())
			continue;

		// Remove object
		m_activeObjects.erase(m_activeObjects.begin() + i);
		--i;

		// Remove networking reference
		if (object->GetNetworkID() != 0)
		{
			m_netObjectLookup.erase(object->GetNetworkID());
			// TODO - Active Session callback
		}

		delete object;
	}
}

#ifdef BUILD_CLIENT
void Game::DisplayUpdate(const float& deltaTime)
{
	if (m_currentLevel != nullptr)
		m_currentLevel->DisplayUpdate(m_engine->GetDisplayWindow(), deltaTime);
}
#endif

void Game::OnGameHooked(Engine* engine)
{
	m_engine = engine;

	// Switch to correct level
	NetSession* session = GetSession();
	if(session == nullptr || !session->IsHost())
		SwitchLevel(m_defaultLevel);
	else
		SwitchLevel(m_defaultNetLevel);
}


void Game::RegisterClass(const MClass* classType) 
{
	// Register level
	if (classType->IsChildOf(LLevel::StaticClass()))
	{
		if (classType == LLevel::StaticClass())
		{
			LOG_ERROR("Cannot register class, as it is base class LLevel!");
			return;
		}
		m_registeredLevels[classType->GetID()] = classType;
	}

	// Register actor
	else if (classType->IsChildOf(AActor::StaticClass()))
	{
		if (classType == AActor::StaticClass())
		{
			LOG_ERROR("Cannot register class, as it is base class AActor!");
			return;
		}
		m_registeredActorTypes[classType->GetID()] = classType;
	}

	// Register object
	else if (classType->IsChildOf(OObject::StaticClass()))
	{
		if (classType == OObject::StaticClass())
		{
			LOG_ERROR("Cannot register class, as it is base class OObject!");
			return;
		}
		m_registeredObjectTypes[classType->GetID()] = classType;
	}

	// Unsupported class type
	else
	{
		LOG_ERROR("Cannot register class '%s', as unknown class type", classType->GetName().c_str());
	}
}


bool Game::SwitchLevel(const SubClassOf<LLevel>& levelType) 
{
	// Close current level
	if (m_currentLevel != nullptr)
	{
		LOG("Closing level '%s'", m_currentLevel->GetClass()->GetName().c_str());
		m_currentLevel->Destroy();
		delete m_currentLevel;
		m_currentLevel = nullptr;
	}

#ifdef BUILD_DEBUG
	// Check level is supported
	if (!IsRegisteredLevel(levelType->GetID()))
	{
		LOG_ERROR("Cannot switch to level '%s', as it is not registered to the game", levelType->GetName().c_str());
		return false;
	}
#endif

#ifdef BUILD_CLIENT
	// Recentre camera
	sf::RenderWindow* window = m_engine->GetDisplayWindow();
	if (window != nullptr)
		window->setView(window->getDefaultView());
#endif

	// Attempt to load desired level
	LOG("Loading level '%s'", levelType->GetName().c_str());
	m_currentLevel = levelType->New<LLevel>();
	m_currentLevel->OnLevelActive(this);
	m_currentLevel->Build();
	return true;
}

bool Game::SwitchLevel(const uint16& levelId) 
{
	// Close current level
	if (m_currentLevel != nullptr)
	{
		LOG("Closing level '%s'", m_currentLevel->GetClass()->GetName().c_str());
		m_currentLevel->Destroy();
		delete m_currentLevel;
		m_currentLevel = nullptr;
	}

#ifdef BUILD_CLIENT
	// Recentre camera
	sf::RenderWindow* window = m_engine->GetDisplayWindow();
	if (window != nullptr)
		window->setView(window->getDefaultView());
#endif

	// Level not registered
	if (!IsRegisteredLevel(levelId))
	{
		LOG_ERROR("Cannot switch to level to id %i, as it is not registered to the game", levelId);
		return false;
	}

	// Attempt to load desired level
	SubClassOf<LLevel>& levelType = m_registeredLevels[levelId];
	LOG("Loading level '%s'", levelType->GetName().c_str());
	m_currentLevel = levelType->New<LLevel>();
	m_currentLevel->OnLevelActive(this);
	m_currentLevel->Build();
	return true;
}

void Game::AddObject(OObject* object)
{
#if BUILD_DEBUG
	// Check class is registered (Assume this is not needed for release)
	if (!IsRegisteredObject(object->GetClass()->GetID()))
		LOG_WARNING("Adding object of class '%s' that is not registered!", object->GetClass()->GetName().c_str());
#endif

	// Add object
	m_activeObjects.emplace_back(object);
	object->OnGameLoaded(this);
	

	// Add to look up table, if net synced
	NetSession* session = GetSession();
	if (session != nullptr && object->GetNetworkID() != 0)
		m_netObjectLookup[object->GetNetworkID()] = object;
}

OObject* Game::SpawnObject(const SubClassOf<OObject>& objectClass)
{
	OObject* object = objectClass->New<OObject>();
	if (object == nullptr)
		return nullptr;
	AddObject(object);
	return object;
}

NetSession* Game::GetSession() const
{ 
	return m_engine->GetNetController()->GetSession(); 
}