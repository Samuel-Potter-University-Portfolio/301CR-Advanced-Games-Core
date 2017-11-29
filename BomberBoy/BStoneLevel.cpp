#include "BStoneLevel.h"


CLASS_SOURCE(LBStoneLevel)


void LBStoneLevel::BuildArena(ABLevelArena* arena) 
{
	const uint32 size = 16;
	arena->ResetArena(uvec2(size, size));

	// Set pattern
	for (uint32 x = 1; x < size - 1; ++x)
		for (uint32 y = 1; y < size - 1; ++y)
		{
			if (x % 3 == 2 && y % 3 == 2)
				arena->SetTile(x, y, ABLevelArena::TileType::Wall);
			else if (x % 3 == 0 && y % 3 == 0)
				arena->SetTile(x, y, ABLevelArena::TileType::LootBox);
			else
				arena->SetTile(x, y, ABLevelArena::TileType::Box);
		}
}