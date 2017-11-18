#include "MainLevel.h"


CLASS_SOURCE(LMainLevel)


#include "Core\Camera.h"
#include "BomberCharacter.h"
#include "BLevelControllerBase.h"

#include "TestEntity.h"


LMainLevel::LMainLevel()
{
	levelControllerClass = ABLevelControllerBase::StaticClass();
}

void LMainLevel::OnBuildLevel()
{
	SpawnActor<ACamera>();

#ifdef BUILD_CLIENT
	// DEBUG: TEST
	if (GetGame()->GetSession() == nullptr)
	{
		GetGame()->GetEngine()->GetNetController()->JoinSession(GetGame()->GetEngine()->GetDefaultNetIdentity());
	}
#endif
}
