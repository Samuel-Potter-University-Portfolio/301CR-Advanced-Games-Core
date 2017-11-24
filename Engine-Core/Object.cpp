#include "Includes\Core\Object.h"
#include "Includes\Core\Game.h"
#include "Includes\Core\NetSession.h"

CLASS_SOURCE(OObject, CORE_API)


OObject::OObject()
{
	bIsDestroyed = false;
}

void OObject::OnGameLoaded(Game* game) 
{
	m_game = game;
	m_name = GetClass()->GetName() + "_instance";
	m_decodingContext = game;
	OnBegin();
}

void OObject::Destroy(OObject* object) 
{
	if (!object->bIsDestroyed)
	{
		object->OnDestroy();

		// Make sure active session knows
		NetSession* session = object->GetGame()->GetSession();
		if (session != nullptr && object->GetNetworkID() != 0)
			session->OnNetObjectDestroy(object);

		object->bIsDestroyed = true;
	}
}