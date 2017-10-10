#include "Includes\Core\Level.h"
#include "Includes\Core\Camera.h"


Level::Level(string name)
{
	m_name = name;
	m_camera = new Camera;
	AddEntity(m_camera);
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
}
