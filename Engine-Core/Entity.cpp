#include "Includes\Core\Entity.h"
#include "Includes\Core\Level.h"


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
	UpdateRole(level->GetGame()->GetSession(), true);
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

#ifdef BUILD_CLIENT
void Entity::Draw(sf::RenderWindow* window, const float& deltaTime) 
{
}
#endif