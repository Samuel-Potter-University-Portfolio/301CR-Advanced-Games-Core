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
	m_id = ++counter;
}

void Level::DestroyLevel()
{

}

void Level::AddEntity(Entity* entity) 
{
	m_entities.push_back(entity);
	entity->m_instanceId = m_entityCounter++;
	entity->HandleSpawn(this);
}