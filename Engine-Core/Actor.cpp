#include "Includes\Core\Actor.h"
#include "Includes\Core\Level.h"
#include "Includes\Core\Game.h"
#include "Includes\Core\Engine.h"


CLASS_SOURCE(AActor, CORE_API)
uint32 AActor::s_instanceCounter = 1;


AActor::AActor() :
	m_instanceId(s_instanceCounter++)
{
	bIsTickable = false;
	m_drawingLayer = 0;
}


bool AActor::RegisterRPCs(const char* func, RPCInfo& outInfo) const
{
	RPC_INDEX_HEADER(func, outInfo);
	return false;
}

bool AActor::ExecuteRPC(uint16& id, ByteBuffer& params)
{
	RPC_EXEC_HEADER(id, params);
	return false;
}

void AActor::RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode)
{
	SYNCVAR_INDEX_HEADER(outQueue, socketType, index, trackIndex, forceEncode);
	SYNCVAR_INDEX(UDP, SyncVarMode::OnChange, vec2, m_netLocation);
}

bool AActor::ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks)
{
	SYNCVAR_EXEC_HEADER(id, value, skipCallbacks);
	SYNCVAR_EXEC_Callback(m_netLocation, OnNetLocationUpdate);
	return false;
}


void AActor::OnLevelLoaded(LLevel* level)
{
	m_level = level;
	OnGameLoaded(level->GetGame());
}

void AActor::OnPostTick()
{
	if (IsNetHost())
		m_netLocation = m_location;
	else if (IsNetOwner())
	{
		// TODO - Send location to server
	}
}


AssetController* AActor::GetAssetController() 
{ 
	return GetGame()->GetAssetController(); 
}

void AActor::OnNetLocationUpdate() 
{
	// Check to see if location is too far out
	if (IsNetOwner())
	{
		const vec2 dif = m_location - m_netLocation;
		const float distanceSqrd = dif.x*dif.x + dif.y*dif.y;
		if(distanceSqrd >= m_catchupDistance * m_catchupDistance)
			m_location = m_netLocation;
	}
	// Forcefully set location
	else
		m_location = m_netLocation;
}