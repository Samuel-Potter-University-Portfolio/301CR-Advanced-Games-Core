#include "Includes\Core\Level.h"


Level::Level(string name)
{
	m_name = name;
}

Level::~Level()
{
	for (Entity* e : m_entities)
		delete e;
}

void Level::HookGame(Game* game) 
{
	static uint32 counter = 0;
	m_game = game;
	m_id = ++counter; // Give each level a unique id
}

void Level::OnPostLoad() 
{
	// Flag all entitys that were loaded in during level build
	for (Entity* e : m_entities)
		e->bWasLoadedWithLevel = true;
}

void Level::DestroyLevel()
{

}

void Level::AddEntity(Entity* entity) 
{
	m_entities.emplace_back(entity);
	entity->m_instanceId = m_entityCounter++;

	NetSession* session = GetGame()->GetSession();

	// Store for faster lookup
	if (session != nullptr)
	{
		if (entity->GetNetworkID() != 0)
			m_netEntities[entity->GetNetworkID()] = entity;
	}
	// Give dud role, so logic still works
	else
		entity->m_netRole = NetRole::HostOwner;

	entity->HandleSpawn(this);
}

Entity* Level::GetEntityFromNetId(const uint16& netId) const
{
	auto it = m_netEntities.find(netId);
	if (it == m_netEntities.end())
		return nullptr;
	else
		return it->second;
}

Entity* Level::GetEntityFromInstanceId(const uint16& id) const
{
	for (int i = 0; i < m_entities.size(); ++i)
		if (m_entities[i]->GetInstanceID() == id)
			return m_entities[i];
	return nullptr;
}