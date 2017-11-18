#include "BLevelControllerBase.h"
#include "BomberCharacter.h"


CLASS_SOURCE(ABLevelControllerBase)


ABLevelControllerBase::ABLevelControllerBase()
{
}


void ABLevelControllerBase::OnPlayerConnect(OPlayerController* player, const bool& newConnection) 
{
	Super::OnPlayerConnect(player, newConnection);


	// Get correct player controller
	OBPlayerController* bomberPlayer = dynamic_cast<OBPlayerController*>(player);
	if (bomberPlayer == nullptr)
	{
		LOG_ERROR("ABLevelControllerBase expects players to be OBPlayerControllers");
		return;
	}
	activePlayers.emplace_back(bomberPlayer);


	// Spawn in character for this controller
	if (IsNetHost())
	{
		ABomberCharacter* character = GetLevel()->SpawnActor<ABomberCharacter>(ABomberCharacter::StaticClass(), player);
		character->SetLocation(vec2(20, 0) * (float)player->GetNetworkOwnerID());
	}
}

void ABLevelControllerBase::OnPlayerDisconnect(OPlayerController* player) 
{
	Super::OnPlayerDisconnect(player);


	// Remove player from active players list
	OBPlayerController* bomberPlayer = dynamic_cast<OBPlayerController*>(player);
	auto it = std::find(activePlayers.begin(), activePlayers.end(), bomberPlayer);
	if (bomberPlayer != nullptr && it != activePlayers.end())
		activePlayers.erase(it);


	// Destroy character, if host
	if (IsNetHost())
	{
		// Destroy the character for that controller
		auto playerCharacters = GetLevel()->GetActiveActors<ABomberCharacter>();

		for (ABomberCharacter* character : playerCharacters)
			if (character->GetNetworkOwnerID() == player->GetNetworkOwnerID())
				Destroy(character);
	}
}