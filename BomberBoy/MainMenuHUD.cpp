#include "MainMenuHUD.h"


CLASS_SOURCE(AMainMenuHUD)


class UTempGUI : public UGUIBase 
{
	CLASS_BODY()
private:
	string str;
public:
	UTempGUI() 
	{
		bIsTickable = true;
	}

protected:
	virtual void OnTick(const float&)
	{
		/*
		string input = GetHUD()->GetInputController()->GetTypedString();
		if (!input.empty())
		{
			str.reserve(str.size() + input.size());
			for (const char& c : input)
			{
				if (c == '\b')
					str.erase(str.size() - 1);
				else if (c == '\r')
					str += '\n';
				else
					str += c;
			}

			LOG("%s", str.c_str());
		}
		*/
	}

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

void AMainMenuHUD::OnBegin()
{
	Super::OnBegin();

	UInputField* elem0 = (UInputField*)AddElement(UInputField::StaticClass());
	elem0->SetAnchor(vec2(-1, -1));
	elem0->SetSize(vec2(200, 40));
	elem0->SetOrigin(vec2(0, 0));
	elem0->SetFont(GetAssetController()->GetFont("Resources\\UI\\coolvetica.ttf"));
	elem0->SetScalingMode(UGUIBase::ScalingMode::PixelPerfect);
	elem0->SetDisabled(true);


	UButton* button = (UButton*)AddElement(UButton::StaticClass());
	button->SetAnchor(vec2(0, 0));
	button->SetSize(vec2(200, 60));
	button->SetOrigin(vec2(100, 30));
	button->SetScalingMode(UGUIBase::ScalingMode::PixelPerfect);
	button->SetFont(GetAssetController()->GetFont("Resources\\UI\\coolvetica.ttf"));
	button->SetCallback(
		[this]() 
		{
			LOG("WOOO");
		}
	);
	button->SetDisabled(true);

	//UGUIBase* elem1 = AddElement(UTempGUI::StaticClass());
	//elem1->SetAnchor(vec2(1, 1));
	//elem1->SetScalingMode(UGUIBase::ScalingMode::Expand);
	//elem1->SetScalingMode(UGUIBase::ScalingMode::PixelPerfect);

	//UInputField* elem2 = (UInputField*)AddElement(UInputField::StaticClass());
	//elem2->SetSize(vec2(100, 30));
	//elem2->SetFont(GetAssetController()->GetFont("Resources\\UI\\Pokemon Classic.ttf"));
	//elem2->SetScalingMode(UGUIBase::ScalingMode::PixelPerfect);
}

