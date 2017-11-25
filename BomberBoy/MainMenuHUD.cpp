#include "MainMenuHUD.h"


CLASS_SOURCE(AMainMenuHUD)




AMainMenuHUD::AMainMenuHUD()
{
	UGUIBase* elem0 = AddElement(UGUIBase::StaticClass());
	elem0->SetAnchor(vec2(-1, -1));
	elem0->SetScalingMode(UGUIBase::ScalingMode::Expand);

	UGUIBase* elem1 = AddElement(UGUIBase::StaticClass());
	elem1->SetAnchor(vec2(1, 1));
	elem1->SetScalingMode(UGUIBase::ScalingMode::PixelPerfect);
}

