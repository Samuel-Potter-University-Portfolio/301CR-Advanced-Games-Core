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

void AActor::OnLevelLoaded(LLevel* level)
{
	m_level = level;
	OnGameLoaded(level->GetGame());
}

TextureAsset AActor::GetTextureAsset(const string path)
{
	Game* game = GetGame();
	if (game != nullptr) // If game is not nullptr, everything else should be fine too
		return game->GetEngine()->GetAssetController()->GetTextureAsset(path);
	else
		return TextureAsset();
}