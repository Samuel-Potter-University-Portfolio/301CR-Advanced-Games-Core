#include "Includes\Core\Level.h"
#include "Includes\Core\Game.h"
#include "Includes\Core\NetSession.h"


CLASS_SOURCE(LLevel, CORE_API)
uint32 LLevel::s_instanceCounter = 0;


LLevel::LLevel() :
	m_instanceId(s_instanceCounter++)
{
}

LLevel::~LLevel()
{
}

void LLevel::OnLevelActive(Game* game)
{
	m_game = game;
}

void LLevel::MainUpdate(const float& deltaTime) 
{
	if (bIsDestroying)
		return;

	for (AActor* actor : m_activeActors)
	{
		if (actor->IsTickable())
		{
			actor->OnTick(deltaTime);
			actor->OnPostTick();
		}
	}


	// Perform cleanup
	for (uint32 i = 0; i < m_activeActors.size(); ++i)
	{
		AActor* actor = m_activeActors[i];
		if (!actor->IsDestroyed())
			continue;

		// Remove object
		m_activeActors.erase(m_activeActors.begin() + i);
		--i;

		// Remove networking reference
		if (actor->GetNetworkID() != 0)
			m_netActorLookup.erase(actor->GetNetworkID());

		delete actor;
	}
}

#ifdef BUILD_CLIENT
void LLevel::DisplayUpdate(sf::RenderWindow* window, const float& deltaTime)
{
	if (bIsDestroying)
		return;

	for (uint32 layer = 0; layer <= 10; ++layer)
	{
		for (uint32 i = 0; i < m_activeActors.size(); ++i)
		{
			AActor* actor = m_activeActors[i]; // TODO - Better safety with memory destruction

			if (actor->GetDrawingLayer() == layer && !actor->IsDestroyed() && actor->IsVisible())
				actor->OnDraw(window, deltaTime);
		}
	}
}
#endif


void LLevel::Build()
{
	AActor::s_instanceCounter = 1;
	bIsBuilding = true;

	m_levelController = SpawnActor<ALevelController>(*levelControllerClass);
	m_levelController->UpdateRole(GetGame()->GetSession());

	OnBuildLevel();

	bIsBuilding = false;

	// Call player connect callback
	auto playerList = GetGame()->GetActiveObjects<OPlayerController>();
	for (OPlayerController* player : playerList)
		m_levelController->OnPlayerConnect(player, false);
}

void LLevel::Destroy()
{
	bIsDestroying = true;
	OnDestroyLevel();
	for (AActor* actor : m_activeActors)
	{
		actor->OnDestroy();
		delete actor;
	}
}

void LLevel::AddActor(AActor* actor)
{
#if BUILD_DEBUG
	// Check class is registered (Assume this is not needed for release)
	if (!GetGame()->IsRegisteredActor(actor->GetClass()->GetID()))
		LOG_WARNING("Adding actor of class '%s' that is not registered!", actor->GetClass()->GetName().c_str());
#endif

	// Add to level
	m_activeActors.emplace_back(actor);

	// Add to look up table, if net synced
	NetSession* session = GetGame()->GetSession();
	if (session != nullptr)
	{
		if (actor->GetNetworkID() != 0)
			m_netActorLookup[actor->GetNetworkID()] = actor;
		actor->UpdateRole(session);
	}
	else
		// Give fake role so offline play still works
		actor->UpdateRole(nullptr, true);

	actor->OnLevelLoaded(this);
}

template<>
AActor* LLevel::SpawnActor(const SubClassOf<AActor>& actorClass, const OObject* owner)
{
	AActor* actor = actorClass->New<AActor>();
	if (actor == nullptr)
		return nullptr;
	actor->bWasSpawnedWithLevel = bIsBuilding;
	if (owner != nullptr)
		actor->m_networkOwnerId = owner->m_networkOwnerId;
	AddActor(actor);
	return actor;
}

AActor* LLevel::GetActorByInstance(const uint32& id) const 
{
	// For each not guaranteed to be safe
	for (uint32 i = 0; i < m_activeActors.size(); ++i)
		if (m_activeActors[i]->GetInstanceID() == id)
			return m_activeActors[i];
	return nullptr;

}

AActor* LLevel::GetActorByNetID(const uint32& id) const 
{
	auto it = m_netActorLookup.find(id);
	if (it == m_netActorLookup.end())
		return nullptr;
	return it->second;
}