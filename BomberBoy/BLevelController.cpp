#include "BLevelController.h"
#include "BCharacter.h"

#include <ctime>

#include "LobbyLevel.h"
#include "APIController.h"


CLASS_SOURCE(ABMatchController)


ABMatchController::ABMatchController()
{
	bIsTickable = true;
}


void ABMatchController::OnPlayerConnect(OPlayerController* player, const bool& newConnection) 
{
	Super::OnPlayerConnect(player, newConnection);


	// Get correct player controller
	OBPlayerController* bomberPlayer = dynamic_cast<OBPlayerController*>(player);
	if (bomberPlayer == nullptr)
	{
		LOG_ERROR("ABMatchController expects players to be OBPlayerControllers");
		return;
	}
	m_activePlayers.emplace_back(bomberPlayer);


	// Spawn in character for this controller
	if (IsNetHost())
	{
		bomberPlayer->AssignColour();
		bomberPlayer->SetReady(false); // Reset ready status
		ABCharacter* character = GetLevel()->SpawnActor<ABCharacter>(ABCharacter::StaticClass(), player);
		character->SetActive(false);
		character->SetColour(bomberPlayer->m_colourIndex);

		bomberPlayer->m_character = character;
		character->m_playerController = bomberPlayer;
	}
}

void ABMatchController::OnPlayerDisconnect(OPlayerController* player) 
{
	Super::OnPlayerDisconnect(player);


	// Remove player from active players list
	OBPlayerController* bomberPlayer = dynamic_cast<OBPlayerController*>(player);
	auto it = std::find(m_activePlayers.begin(), m_activePlayers.end(), bomberPlayer);
	if (bomberPlayer != nullptr && it != m_activePlayers.end())
		m_activePlayers.erase(it);


	// Destroy character, if host
	if (IsNetHost())
	{
		// Destroy the character for that controller
		auto playerCharacters = GetLevel()->GetActiveActors<ABCharacter>();

		for (ABCharacter* character : playerCharacters)
			if (character->GetNetworkOwnerID() == player->GetNetworkOwnerID())
				Destroy(character);
	}
}


void ABMatchController::OnBegin() 
{
	if (!IsNetHost())
		return;

	m_currentState = MatchState::InActive;
	m_stateTimer = 60.0f;
}

void ABMatchController::OnTick(const float& deltaTime) 
{
	if (!IsNetHost())
		return;

	switch (m_currentState)
	{
		case MatchState::InActive:
		{
			// If enough players are currently connected, start countdown
			if (m_activePlayers.size() != 0)
			{
				m_matchStartEpoch = std::time(nullptr);
				m_currentState = MatchState::StartingRound;
				m_stateTimer = 5.0f;
				return;
			}

			// Switch back to lobby if been inactive for too long
			m_stateTimer -= deltaTime;
			if (m_stateTimer < 0.0f)
			{
				LOG("Switching back to lobby (Been idle for too long)");
				GetGame()->SwitchLevel(LLobbyLevel::StaticClass());
			}
			break;
		}

		case MatchState::StartingRound:
		{
			// Wait a bit until starting the round
			m_stateTimer -= deltaTime;
			if (m_stateTimer < 0.0f)
			{
				// Spawn in all players in random locations
				ABLevelArena* arena = GetLevel()->GetFirstActor<ABLevelArena>();
				const std::vector<ivec2>& spawns = arena->GetSpawnPoints();

				for (OBPlayerController* player : m_activePlayers)
				{
					player->m_character->SpawnAtTile(spawns[rand() % spawns.size()]);
					player->m_character->SetActive(true);
				}
				arena->ResetArenaState();

				m_currentState = MatchState::InRound;
				LOG("Starting round %i", m_roundCounter);
			}
			break;
		}

		case MatchState::InRound:
		{
			// Go inactive if players aren't connected
			if (m_activePlayers.size() == 0)
			{
				m_currentState = MatchState::InActive;
				return;
			}
			// Don't run gamemode, if not enough players are connected
			else if (m_activePlayers.size() == 1)
				return;


			// Check to see how many players alive there are
			OBPlayerController* winner = nullptr;
			uint32 liveCount = 0;
			for (OBPlayerController* player : m_activePlayers)
			{
				if (!player->m_character->bIsDead)
				{
					// Found more than one winner...
					if (++liveCount == 2)
						break;
					winner = player;
				}
			}

			// Last person alive is the winner
			if (liveCount == 1)
			{
				LOG("'%s' won the round!", winner->GetPlayerName().c_str());
				++winner->m_character->m_roundWins;

				m_currentState = MatchState::EndOfRound;
				m_stateTimer = 3.0f;
			}
			// No-one alive (Must have killed each other or suicide pact)??
			else if (liveCount == 0)
			{
				LOG("This round has no winner..");
				m_currentState = MatchState::EndOfRound;
			}
			// TODO - Broadcast to clients

			break;
		}

		case MatchState::EndOfRound:
		{
			// Check if a player has wons
			for (OBPlayerController* player : m_activePlayers)
			{
				if (player->m_character->GetRoundsWon() >= m_roundWinAmount)
				{
					LOG("Player '%s' has won the match!", player->GetPlayerName().c_str());
					m_currentState = MatchState::EndOfMatch;
					m_stateTimer = 5.0f;

					// Send match data to API
				#ifdef API_SUPPORTED
					OAPIController* apiController = GetGame()->GetFirstObject<OAPIController>();
					if (apiController != nullptr)
						apiController->ReportMatchResults(m_matchStartEpoch, m_activePlayers);
				#endif

					// TODO - Broadcast to clients
					return;
				}
			}


			// Wait a bit until starting the round
			m_stateTimer -= deltaTime;
			if (m_stateTimer < 0.0f)
			{
				// Reset arena
				m_roundCounter++;
				m_currentState = MatchState::StartingRound;
				m_stateTimer = 3.0f;

				// Make sure all characters are dead
				for (OBPlayerController* player : m_activePlayers)
				{
					if (!player->m_character->bIsDead)
					{
						player->m_character->bIsDead = true;
						player->m_character->SetActive(false);
					}
				}
			}


			break;
		}

		case MatchState::EndOfMatch:
		{
			// Once timer is over, switch back to lobby
			m_stateTimer -= deltaTime;
			if (m_stateTimer < 0.0f)
				GetGame()->SwitchLevel(GetGame()->defaultNetLevel);
			break;
		}
	}
}

void ABMatchController::OnPlayerExploded(ABCharacter* victim, ABCharacter* killer) 
{
	// Don't care about being in explosion if round is not in progress
	if (m_currentState != MatchState::InRound)
		return;

	victim->bIsDead = true;
	victim->SetActive(false);
	LOG("'%s' was killed by '%s'", victim->m_playerController->GetDisplayName().c_str(), (killer == nullptr ? "<Unknown>" : killer->m_playerController->GetDisplayName().c_str()));

	// TODO - Broadcast to all players

	// Update stats
		++victim->m_deaths;
	if (victim == killer) // Lose a kill because suicide
		--victim->m_kills;
	else if(killer != nullptr)
		++killer->m_kills;
}