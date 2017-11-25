#include "MainMenuHUD.h"


CLASS_SOURCE(AMainMenuHUD)


class UTempGUI : public UGUIBase 
{
	CLASS_BODY()

protected:
	virtual void OnMouseOver() override
	{
	}

	virtual void OnMouseEnter() override 
	{
		SetColour(Colour::Green);
		LOG("Enter")
	}

	virtual void OnMouseExit() override 
	{
		SetColour(Colour::Red);
		LOG("Exit")
	}

	virtual void OnMousePressed() override 
	{
		SetColour(Colour::Green);
		LOG("Press")
	}

	virtual void OnMouseReleased() override 
	{
		LOG("Released")
	}
};
CLASS_SOURCE(UTempGUI)


AMainMenuHUD::AMainMenuHUD()
{
	UGUIBase* elem0 = AddElement(UTempGUI::StaticClass());
	elem0->SetAnchor(vec2(-1, -1));
	elem0->SetScalingMode(UGUIBase::ScalingMode::Expand);

	UGUIBase* elem1 = AddElement(UTempGUI::StaticClass());
	elem1->SetAnchor(vec2(1, 1));
	//elem1->SetScalingMode(UGUIBase::ScalingMode::Expand);
	elem1->SetScalingMode(UGUIBase::ScalingMode::PixelPerfect);
}

