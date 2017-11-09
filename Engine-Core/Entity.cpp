#include "Includes\Core\Entity.h"


Entity::Entity()
{
	m_name = "Unnamed Entity";
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
	// Clear any net data, ready for this update
	ClearQueuedNetData();

	if (bHasStarted)
		OnTick(deltaTime);
	else
	{
		OnBegin();
		bHasStarted = true;
	}
}

#ifdef BUILD_CLIENT
void Entity::Draw(sf::RenderWindow* window, const float& deltaTime) 
{
}
#endif