#include "Includes\Core\Object.h"


CLASS_SOURCE(OObject, CORE_API)


OObject::OObject()
{
	bIsDestroyed = false;
}

void OObject::OnGameLoaded(Game* game) 
{
	m_game = game;
	m_name = GetClass()->GetName() + "_instance";
	OnBegin();
}

void OObject::Destroy(OObject* object) 
{
	if (!object->bIsDestroyed)
	{
		object->OnDestroy();
		object->bIsDestroyed = true;
	}
}