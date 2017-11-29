#include "BLevelController.h"
#include "BCharacter.h"

#include "LobbyLevel.h"


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
		ABCharacter* character = GetLevel()->SpawnActor<ABCharacter>(ABCharacter::StaticClass(), player);
		character->SetActive(false);
		character->m_colourIndex = bomberPlayer->m_colourIndex;
		character->UpdateColour();
		bomberPlayer->m_character = character;
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

				m_currentState = MatchState::InRound;
			}
			break;
		}

		case MatchState::InRound:
		{
			// Check to see how many players alive there are
			break;
		}

		case MatchState::EndOfRound:
		{
			break;
		}

		case MatchState::EndOfMatch:
		{
			break;
		}
	}
}