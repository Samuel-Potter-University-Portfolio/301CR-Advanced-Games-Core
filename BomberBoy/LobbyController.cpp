#include "LobbyController.h"

#include "MainLevel.h"

#include "LobbyHUD.h"


CLASS_SOURCE(ALobbyController)
const std::vector<SubClassOf<LLevel>> ALobbyController::s_supportedLevels(
{
	LMainLevel::StaticClass()
});


ALobbyController::ALobbyController()
{
	bIsTickable = true;
}

bool ALobbyController::RegisterRPCs(const char* func, RPCInfo& outInfo) const 
{
	RPC_INDEX_HEADER(func, outInfo);
	return false;
}
bool ALobbyController::ExecuteRPC(uint16& id, ByteBuffer& params) 
{
	RPC_EXEC_HEADER(id, params);
	return false;
}

void ALobbyController::RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode) 
{
	SYNCVAR_INDEX_HEADER(outQueue, socketType, index, trackIndex, forceEncode);
	SYNCVAR_INDEX(UDP, SyncVarMode::Always, float, m_logicTimer);
	SYNCVAR_INDEX(TCP, SyncVarMode::OnChange, bool, bIsTimerActive);	
	SYNCVAR_INDEX(UDP, SyncVarMode::Always, PlayerVoteMap, m_mapVotes);
}
bool ALobbyController::ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks)
{
	SYNCVAR_EXEC_HEADER(id, value, skipCallbacks);
	SYNCVAR_EXEC(m_logicTimer);
	SYNCVAR_EXEC(bIsTimerActive);
	SYNCVAR_EXEC(m_mapVotes);
	return false;
}

void ALobbyController::OnBegin() 
{
	Super::OnBegin();

	NetSession* session = GetGame()->GetSession();
	if (session != nullptr)
		m_maxPlayers = session->GetMaxPlayerCount();
	else
		m_maxPlayers = 2;
}

void ALobbyController::OnTick(const float& deltaTime)
{
	Super::OnTick(deltaTime);



	// Only tick past here, if host
	if (!IsNetHost())
		return;


	// Count down timer
	if (bIsTimerActive)
	{
		m_totalTimer -= deltaTime;
		if (m_totalTimer < 0.0f)
			m_totalTimer = 0.0f;

		m_logicTimer -= deltaTime;
		if (m_logicTimer < 0.0f)
			m_logicTimer = 0.0f;
	}


	// Don't start timer until have at least 2 players
	if (m_players.size() < 2)
	{
		if (bIsTimerActive)
		{
			m_totalTimer = 0.0f;
			bIsTimerActive = false;
		}
		return;
	}
	else
	{
		// Start timer because we have enough players
		if (!bIsTimerActive)
		{
			// Wait 4 minutes for more players to join
			m_totalTimer = 240.0f;
			m_logicTimer = m_totalTimer;
			bIsTimerActive = true;
		}
	}



	// If a third of players are ready, start final start count down
	const uint32 requiredReadies = std::max(m_players.size() / 3, 2U);
	uint32 readyCount = 0;
	for (auto player : m_players)
		if (player->IsReady())
			++readyCount;


	// 1/3 of players are ready
	if (readyCount >= requiredReadies)
	{
		if (!bIsLaunching)
		{
			// Clamp logic timer to under 25 seconds (Speed match start)
			bIsLaunching = true;
			m_logicTimer = m_logicTimer > 25.0f ? 25.0f : m_logicTimer;
		}
	}
	// Not enough players ready
	else 
	{
		if (bIsLaunching)
		{
			// Reset timer to actual value, as we're not currently trying to start
			bIsLaunching = false;
			m_logicTimer = m_totalTimer;
		}
	}


	// Load the map with the most votes
	if (m_logicTimer == 0.0f)
	{
		uint32 levelIndex = 0;
		uint32 levelVotes = 0;
		
		for (uint32 i = 0; i < s_supportedLevels.size(); ++i)
		{
			uint32 votes = GetMapVotes(i);
			if (votes > levelVotes)
			{
				levelIndex = i;
				levelVotes = votes;
			}
		}
		GetGame()->SwitchLevel(s_supportedLevels[levelIndex]);
	}
}


void ALobbyController::OnPlayerConnect(OPlayerController* player, const bool& newConnection) 
{
	Super::OnPlayerConnect(player, newConnection);

	// Store player
	OBPlayerController* bplayer = dynamic_cast<OBPlayerController*>(player);
	if (bplayer == nullptr)
		return;
	m_players.emplace_back(bplayer);


	// Update lobby HUD
	ALobbyHUD* hud = dynamic_cast<ALobbyHUD*>(GetLevel()->GetHUD());
	if (hud != nullptr)
		hud->OnPlayerConnect(bplayer);
}

void ALobbyController::OnPlayerDisconnect(OPlayerController* player) 
{
	Super::OnPlayerDisconnect(player);

	// Remove player
	OBPlayerController* bplayer = dynamic_cast<OBPlayerController*>(player);
	if (bplayer == nullptr)
		return;
	m_players.erase(std::remove(m_players.begin(), m_players.end(), bplayer), m_players.end());
	m_mapVotes.erase(bplayer->GetNetworkID());

	// Update lobby HUD
	ALobbyHUD* hud = dynamic_cast<ALobbyHUD*>(GetLevel()->GetHUD());
	if (hud != nullptr)
		hud->OnPlayerDisconnect(bplayer);
}


uint32 ALobbyController::GetMapVotes(const uint32& mapIndex) const
{
	uint32 count = 0;
	for (auto it : m_mapVotes)
		if (it.second == mapIndex)
			++count;
	return count;
}
