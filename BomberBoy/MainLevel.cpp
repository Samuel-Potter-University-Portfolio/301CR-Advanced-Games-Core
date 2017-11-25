#include "MainLevel.h"


CLASS_SOURCE(LMainLevel)


#include "Core\Camera.h"
#include "BCharacter.h"

#include "MainMenuHUD.h"
#include "BLevelControllerBase.h"
#include "BLevelArena.h"


LMainLevel::LMainLevel()
{
	//hudClass = AMainMenuHUD::StaticClass();
	levelControllerClass = ABLevelControllerBase::StaticClass();
}

void LMainLevel::OnBuildLevel()
{
	SpawnActor<ACamera>();
	SpawnActor<ABLevelArena>();
}
