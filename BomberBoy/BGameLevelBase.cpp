#include "BGameLevelBase.h"

#include "Core\Camera.h"
#include "BLevelController.h"

CLASS_SOURCE(LBGameLevelBase)


LBGameLevelBase::LBGameLevelBase()
{
	levelControllerClass = ABMatchController::StaticClass();
}

void LBGameLevelBase::OnBuildLevel() 
{
	SpawnActor<ACamera>();

	ABLevelArena* arena = SpawnActor<ABLevelArena>();
	BuildArena(arena);

	// Clear spawn areas
	for (ivec2 spawn : arena->GetSpawnPoints())
	{
		arena->SetTile(spawn.x + 0, spawn.y + 0, ABLevelArena::TileType::Floor);
		arena->SetTile(spawn.x + 0, spawn.y + 1, ABLevelArena::TileType::Floor);
		arena->SetTile(spawn.x + 1, spawn.y + 0, ABLevelArena::TileType::Floor);
		arena->SetTile(spawn.x + 1, spawn.y + 1, ABLevelArena::TileType::Floor);
	}

	arena->SetDefaultArenaState();
}