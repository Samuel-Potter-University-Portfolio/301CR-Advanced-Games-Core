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

	UInputField* elem0 = AddElement<UInputField>();
	elem0->SetAnchor(vec2(-1, -1));
	elem0->SetSize(vec2(200, 40));
	elem0->SetOrigin(vec2(0, 0));
	elem0->SetFont(GetAssetController()->GetFont("Resources\\UI\\coolvetica.ttf"));
	elem0->SetScalingMode(UGUIBase::ScalingMode::PixelPerfect);
	elem0->SetCallback(
		[elem0](string value)
		{
			LOG("v: '%s'", value.c_str());
			//elem0->SetText("");
		}
	);


	UButton* button = AddElement<UButton>();
	button->SetAnchor(vec2(0, 0));
	button->SetSize(vec2(200, 60));
	button->SetOrigin(vec2(100, 30));
	button->SetScalingMode(UGUIBase::ScalingMode::PixelPerfect);
	button->SetFontSize(30);
	button->SetText("Button");
	button->SetFont(GetAssetController()->GetFont("Resources\\UI\\coolvetica.ttf"));
	button->SetCallback(
		[button]()
		{
			button->SetDisabled(true);
			LOG("WOOO");
		}
	);

	/*
	ULabel* label = AddElement<ULabel>();
	label->SetFont(GetAssetController()->GetFont("Resources\\UI\\coolvetica.ttf"));
	label->SetText("Hello World");
	//label->SetSize(vec2(300, 60));
	label->SetSize(vec2(200, 60));
	label->SetOrigin(vec2(100, 30));
	label->SetHorizontalAlignment(ULabel::HorizontalAlignment::Right);
	label->SetVerticalAlignment(ULabel::VerticalAlignment::Top);
	//label->SetScalingMode(UGUIBase::ScalingMode::Expand);
	label->SetTextColour(Colour::Red);
	label->SetDrawBackground(true);
	*/
}

