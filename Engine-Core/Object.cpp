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


template<>
bool CORE_API Decode<OObjectPtr>(ByteBuffer& buffer, OObjectPtr& out, void* context)
{
	out = nullptr;
	uint16 id;
	if (!Decode<uint16>(buffer, id))
		return false;

	Game* game = (Game*)context;
	if (game == nullptr)
	{
		LOG_ERROR("Cannot decode 'OObject*' without Game* as context");
		return true; // Decode was fine, just invalid context used
	}

	out = game->GetObjectByNetID(id);
	return true;
}