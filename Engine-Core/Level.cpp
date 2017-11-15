#include "Includes\Core\Level.h"
#include "Includes\Core\Game.h"


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
			actor->OnTick(deltaTime);
	}


	// Perform cleanup
	for (uint32 i = 0; i < m_activeActors.size(); ++i)
	{
		AActor*& actor = m_activeActors[i];
		if (!actor->IsDestroyed())
			continue;

		// Remove object
		m_activeActors.erase(m_activeActors.begin() + i);
		--i;

		// Remove networking reference
		if (actor->GetNetworkID() != 0)
		{
			m_netActorLookup.erase(actor->GetNetworkID());
			// TODO - Active Session callback
		}

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
	OnBuildLevel();
	bIsBuilding = false;
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
	actor->OnLevelLoaded(this);

	// Add to look up table, if net synced
	NetSession* session = GetGame()->GetSession();
	if (session != nullptr && actor->GetNetworkID() != 0)
		m_netActorLookup[actor->GetNetworkID()] = actor;
}

AActor* LLevel::SpawnActor(const SubClassOf<AActor>& actorClass, const vec2& location)
{
	AActor* actor = actorClass->New<AActor>();
	if (actor == nullptr)
		return nullptr;
	actor->m_location = location;
	actor->bWasSpawnedWithLevel = bIsBuilding;
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