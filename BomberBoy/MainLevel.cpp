#include "MainLevel.h"
CLASS_SOURCE(LMainLevel)

#include "Core\Camera.h"
#include "TestEntity.h"


LMainLevel::LMainLevel()
{


}

void LMainLevel::OnBuildLevel()
{
	SpawnActor<ACamera>();
	SpawnActor<ATestEntity>();
	SpawnActor<ATestEntity>(vec2(0, 100));
}
