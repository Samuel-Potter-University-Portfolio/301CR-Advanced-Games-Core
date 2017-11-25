#include "Includes\Core\Level.h"
#include "Includes\Core\Game.h"
#include "Includes\Core\NetSession.h"


CLASS_SOURCE(LLevel, CORE_API)
uint32 LLevel::s_instanceCounter = 0;


LLevel::LLevel() :
	m_instanceId(s_instanceCounter++)
{
	// Work around: TODO - Proper memory management to stop ptr errors
	m_drawnActors.reserve(200); 
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
		// Ignore destroyed (Will deal with them after logic tick)
		if (actor->IsDestroyed())
			continue;

		// Tick actors
		if (actor->IsTickable())
			actor->OnTick(deltaTime);

		// Call post tick for all actors (So that they may sync any vars)
		actor->OnPostTick();
	}


	// Perform cleanup
	for (uint32 i = 0; i < m_activeActors.size(); ++i)
	{
		AActor* actor = m_activeActors[i];
		// Attempt to destroy object, if only remaining reference is this
		if (!actor->IsDestroyed() || actor->GetRemainingSystemReferences() != 1)
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
	// Draw all actors by layer
	for (uint32 layer = 0; layer <= 10; ++layer)
	{
		for (uint32 i = 0; i < m_drawnActors.size(); ++i)
		{
			if (bIsDestroying)
				return;

			AActor* actor = m_drawnActors[i]; // TODO - Better safety with memory destruction
			actor->bIsBeingDrawn = true;

			// Remove references
			if (actor->IsDestroyed())
			{
				actor->RemoveSystemReference();
				m_drawnActors.erase(m_drawnActors.begin() + i);
				--i;
				actor->bIsBeingDrawn = false;
				continue; 
			}


			if (actor->GetDrawingLayer() == layer && actor->IsVisible())
				actor->OnDraw(window, deltaTime);
			actor->bIsBeingDrawn = false;
		}
	}

	// Draw hud
	if(m_hud != nullptr)
		m_hud->DisplayUpdate(window, deltaTime);
}
#endif


void LLevel::Build()
{
	AActor::s_instanceCounter = 1;
	bIsBuilding = true;

	// Spawn level controller while building to give unique id between all clients
	m_levelController = SpawnActor<ALevelController>(*levelControllerClass);
	m_levelController->UpdateRole(GetGame()->GetSession());

	OnBuildLevel();

	bIsBuilding = false;


	// Spawn hud after building to not conflict with unique ids
#ifdef BUILD_CLIENT
	m_hud = SpawnActor<AHUD>(*hudClass);
	m_hud->UpdateRole(GetGame()->GetSession());
#endif

	
	// Call player connect callback for any players who are already here
	auto playerList = GetGame()->GetActiveObjects<OPlayerController>();
	for (OPlayerController* player : playerList)
		m_levelController->OnPlayerConnect(player, false);
}

void LLevel::Destroy()
{
	bIsDestroying = true;
	OnDestroyLevel();

	m_drawnActors.clear();

	for (AActor* actor : m_activeActors)
	{
		actor->OnDestroy();

#ifdef BUILD_CLIENT
		// Wait until actor is finished being drawn
		while (actor->bIsBeingDrawn) 
			sf::sleep(sf::milliseconds(2));
#endif

		delete actor;
	}
	m_activeActors.clear();
}

void LLevel::AddActor(AActor* actor)
{
#if BUILD_DEBUG
	// Check class is registered (Assume this is not needed for release)
	if (!GetGame()->IsRegisteredActor(actor->GetClass()->GetID()))
		LOG_WARNING("Adding actor of class '%s' that is not registered!", actor->GetClass()->GetName().c_str());
#endif

	// Add to level
	actor->AddSystemReference();
	m_activeActors.emplace_back(actor);

#ifdef BUILD_CLIENT
	// Add to rendering
	actor->AddSystemReference();
	m_drawnActors.emplace_back(actor);
#endif

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