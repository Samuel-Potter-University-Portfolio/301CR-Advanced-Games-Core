#include "Includes\Core\Entity.h"


using namespace EngineCore;


Entity::Entity(string name)
{
	m_name = name;
}

Entity::~Entity()
{
}

void Entity::HandleSpawn(Level* level) 
{
	m_level = level;
}

void Entity::HandleMainUpdate(const float& deltaTime) 
{
	if (bHasStarted)
		OnTick(deltaTime);
	else
	{
		OnBegin();
		bHasStarted = true;
	}
}