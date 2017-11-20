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
	RPC_INDEX(UDP, RPCCallingMode::Host, SendLocationToHost);
	RPC_INDEX(UDP, RPCCallingMode::Owner, SendLocationToOwner);
	return false;
}

bool AActor::ExecuteRPC(uint16& id, ByteBuffer& params)
{
	RPC_EXEC_HEADER(id, params);
	RPC_EXEC_TwoParam(SendLocationToHost, vec2, uint8);
	RPC_EXEC_TwoParam(SendLocationToOwner, vec2, uint8);
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
	SYNCVAR_EXEC(m_netLocation);
	return false;
}


void AActor::OnLevelLoaded(LLevel* level)
{
	m_level = level;
	OnGameLoaded(level->GetGame());
}

void AActor::OnBegin() 
{
	// Start at correct location
	if (IsNetSynced())
	{
		if (IsNetHost())
			bLocationUpdated = true;
		else
		{
			m_desiredLocation = m_netLocation;
			bLocationUpdated = false;
		}
	}
}

void AActor::OnPostTick()
{
	if (GetNetworkID() != 0)
	{
		// Update net location (So clients receive the correct value)
		if (IsNetHost())
		{
			if (bLocationUpdated && !IsNetOwner())
			{
				++m_netLocationCheckId;
				CallRPC_TwoParam(this, SendLocationToOwner, m_desiredLocation, m_netLocationCheckId);
			}
			m_netLocation = m_desiredLocation;
		}

		// Send location to server
		else if (IsNetOwner())
		{
			if (bLocationUpdated)
			{
				CallRPC_TwoParam(this, SendLocationToHost, m_desiredLocation, m_netLocationCheckId);
			}
		}

		// Not owner
		else
		{
			const vec2 dif = m_desiredLocation - m_netLocation;
			const float distanceSqrd = dif.x*dif.x + dif.y*dif.y;

			// Snap to location
			if (distanceSqrd >= m_netCatchupDistance * m_netCatchupDistance)
				m_desiredLocation = m_netLocation;

			// Close, so smoothly reach position
			else
				m_desiredLocation = m_netLocation*m_netCatchupRate + m_desiredLocation*(1.0f - m_netCatchupRate);
		}

		bLocationUpdated = false;
	}
}


AssetController* AActor::GetAssetController() 
{ 
	return GetGame()->GetAssetController(); 
}

void AActor::SendLocationToHost(const vec2& location, const uint8& checkId)
{
	// Owner hasn't acknowledged previous server sent location set yet
	//if (m_netLocationCheckId != checkId)
	//	return;

	m_desiredLocation = location;
	// TODO - Checks (Can the player actually be here?)
	// If not call RPC telling them where they should be
}

void AActor::SendLocationToOwner(const vec2& location, const uint8& checkId)
{
	m_desiredLocation = location;
	m_netLocationCheckId = checkId;
}