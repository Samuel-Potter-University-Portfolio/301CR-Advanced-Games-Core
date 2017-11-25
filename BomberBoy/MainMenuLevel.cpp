#include "MainMenuLevel.h"

#include "Core\Camera.h"
#include "MainMenuHUD.h"


CLASS_SOURCE(LMainMenuLevel)


LMainMenuLevel::LMainMenuLevel()
{
	hudClass = AMainMenuHUD::StaticClass();
}

void LMainMenuLevel::OnBuildLevel() 
{
	SpawnActor<ACamera>();
}