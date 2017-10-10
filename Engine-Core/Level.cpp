#include "Includes\Core\Level.h"


Level::Level(string name)
{
	m_name = name;
}

Level::~Level()
{
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
}
