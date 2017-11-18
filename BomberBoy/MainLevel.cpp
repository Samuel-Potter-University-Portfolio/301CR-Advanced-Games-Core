#include "MainLevel.h"
CLASS_SOURCE(LMainLevel)

#include "Core\Camera.h"
#include "BomberCharacter.h"


LMainLevel::LMainLevel()
{


}

void LMainLevel::OnBuildLevel()
{
	SpawnActor<ACamera>();
	SpawnActor<ABomberCharacter>();
	//SpawnActor<ATestEntity>(vec2(0, 100));
}
