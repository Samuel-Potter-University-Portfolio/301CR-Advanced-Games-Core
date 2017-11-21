#include "BTileableActor.h"
#include "BLevelArena.h"


CLASS_SOURCE(ABTileableActor)


ABTileableActor::ABTileableActor()
{
}


bool ABTileableActor::RegisterRPCs(const char* func, RPCInfo& outInfo) const
{
	RPC_INDEX_HEADER(func, outInfo);
	RPC_INDEX(UDP, RPCCallingMode::Host, UpdateNetMoveState);
	return false;
}
bool ABTileableActor::ExecuteRPC(uint16& id, ByteBuffer& params)
{
	RPC_EXEC_HEADER(id, params);
	RPC_EXEC_TwoParam(UpdateNetMoveState, Direction, bool);
	return false;
}

void ABTileableActor::RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode)
{
	SYNCVAR_INDEX_HEADER(outQueue, socketType, index, trackIndex, forceEncode);
	SYNCVAR_INDEX(UDP, SyncVarMode::OnChange, Direction, m_netDirection);
	SYNCVAR_INDEX(UDP, SyncVarMode::OnChange, bool, bNetIsMoving);
}
bool ABTileableActor::ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks)
{
	SYNCVAR_EXEC_HEADER(id, value, skipCallbacks);
	SYNCVAR_EXEC(m_netDirection);
	SYNCVAR_EXEC(bNetIsMoving);
	return false;
}


void ABTileableActor::OnTick(const float& deltaTime) 
{
	Super::OnTick(deltaTime);

	// Make sure arena is not null
	if (m_arena == nullptr)
	{
		m_arena = GetLevel()->GetFirstActor<ABLevelArena>();
		if (m_arena == nullptr) // Still no (no arena exists), so exit out
			return;
		m_tileLocation = m_arena->WorldToTile(GetLocation());
	}


	// Don't do checks if not net owner
	if (!IsNetOwner())
	{
		m_tileLocation = m_arena->WorldToTile(GetLocation());
		return;
	}


	// Animation to location
	if (bIsMoving)
	{
		m_movementCooldown -= deltaTime;

		ivec2 endLocation;
		switch (m_direction)
		{
			case ABTileableActor::Up:
				endLocation = m_tileLocation + ivec2(0, -1); // Inverted
				break;
			case ABTileableActor::Down:
				endLocation = m_tileLocation + ivec2(0, 1); // Inverted
				break;
			case ABTileableActor::Left:
				endLocation = m_tileLocation + ivec2(-1, 0);
				break;
			case ABTileableActor::Right:
				endLocation = m_tileLocation + ivec2(1, 0);
				break;
			default:
				endLocation = m_tileLocation;
				break;
		}

		// Finished so reset
		if (m_movementCooldown < 0.0f)
		{
			bIsMoving = false;
			m_tileLocation = endLocation;
			SetLocation(m_arena->TileToWorld(m_tileLocation));
			CallRPC_TwoParam(this, UpdateNetMoveState, m_direction, false);
		}
		// Animation
		else 
		{
			const float t = m_movementCooldown / m_movementSpeed;
			SetLocation(m_arena->TileToWorld(m_tileLocation)*t + m_arena->TileToWorld(endLocation)*(1.0f - t));
		}
	}
}


bool ABTileableActor::AttemptMove(const Direction& dir) 
{
	if (bIsMoving || m_arena == nullptr || !IsNetOwner())
		return false;

	ivec2 destination;
	switch (dir)
	{
		case Direction::Up:
			destination = m_tileLocation + ivec2(0, -1); // Inverted
			break;
		case Direction::Down:
			destination = m_tileLocation + ivec2(0, 1); // Inverted
			break;
		case Direction::Left:
			destination = m_tileLocation + ivec2(-1, 0);
			break;
		case Direction::Right:
			destination = m_tileLocation + ivec2(1, 0);
			break;
	}

	// Make sure can walk here
	auto tew = m_arena->GetTile(destination.x, destination.y);
	if (m_arena->GetTile(destination.x, destination.y) != ABLevelArena::TileType::Floor)
	{
		m_direction = dir;
		CallRPC_TwoParam(this, UpdateNetMoveState, m_direction, false);
		return false;
	}

	bIsMoving = true;
	m_direction = dir;
	m_movementCooldown = m_movementSpeed;
	CallRPC_TwoParam(this, UpdateNetMoveState, m_direction, true);
}

void ABTileableActor::SetTileLocation(const ivec2& tile) 
{
	m_tileLocation = tile;
	if (m_arena != nullptr)
		SetLocation(m_arena->TileToWorld(tile));
}