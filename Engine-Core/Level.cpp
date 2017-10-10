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
	m_game = game;
}

void Level::DestroyLevel()
{

}

void Level::AddEntity(Entity* entity) 
{
	m_entities.push_back(entity);
	entity->HandleSpawn(this);
}