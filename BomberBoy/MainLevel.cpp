#include "MainLevel.h"


CLASS_SOURCE(LMainLevel)


#include "Core\Camera.h"
#include "BCharacter.h"

#include "MainMenuHUD.h"
#include "BLevelControllerBase.h"
#include "BLevelArena.h"


LMainLevel::LMainLevel()
{
	hudClass = AMainMenuHUD::StaticClass();
	levelControllerClass = ABLevelControllerBase::StaticClass();
}

void LMainLevel::OnBuildLevel()
{
	SpawnActor<ACamera>();
	SpawnActor<ABLevelArena>();

#ifdef BUILD_CLIENT
	// DEBUG: TEST
	if (GetGame()->GetSession() == nullptr)
	{
		GetGame()->GetEngine()->GetNetController()->JoinSession(GetGame()->GetEngine()->GetDefaultNetIdentity(), 
			[](NetLayer* layerPtr) 
			{
				DefaultNetLayer* layer = dynamic_cast<DefaultNetLayer*>(layerPtr);
				if (layer == nullptr)
					return;

				layer->SetPassword("Test");
			}
		);
	}
#endif
}
