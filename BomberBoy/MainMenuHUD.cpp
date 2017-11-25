#include "MainMenuHUD.h"
#include "MainLevel.h"


CLASS_SOURCE(AMainMenuHUD)


void AMainMenuHUD::OnBegin()
{
	Super::OnBegin();
	const sf::Font* defaultFont = GetAssetController()->GetFont("Resources\\UI\\coolvetica.ttf");
	const ULabel::ScalingMode& defaultScaling = ULabel::ScalingMode::Expand;

	// Title
	{
		ULabel* title = AddElement<ULabel>();
		title->SetFont(defaultFont);
		title->SetScalingMode(defaultScaling);
		title->SetText("Bomber Boy");
		title->SetTextColour(Colour::Blue);
		title->SetFontSize(100);
		title->SetHorizontalAlignment(ULabel::HorizontalAlignment::Centre);
		title->SetVerticalAlignment(ULabel::VerticalAlignment::Top);

		title->SetLocation(vec2(0, 30));
		title->SetOrigin(vec2(5, 5));
		title->SetSize(vec2(10, 10));
		title->SetAnchor(vec2(0, -1));


		ULabel* icon = AddElement<ULabel>();
		icon->SetScalingMode(defaultScaling);
		icon->SetTexture(GetAssetController()->GetTexture("Resources\\Items\\Default_Bomb.png.0"));
		icon->SetDrawBackground(true);

		icon->SetLocation(vec2(-156, 110));
		icon->SetSize(vec2(80, 80));
		icon->SetOrigin(vec2(50, 50));
		icon->SetAnchor(vec2(0, -1));
	}

	// Left nav menu
	{
		uint32 buttonIndex = 0;
		auto makeButton = 
		[this, defaultFont, defaultScaling, buttonIndex]() mutable -> UButton*
		{
			uint32& b = buttonIndex;
			UButton* button = AddElement<UButton>();
			button->SetFont(defaultFont);
			button->SetScalingMode(defaultScaling);
			button->SetVerticalAlignment(ULabel::VerticalAlignment::Top);

			button->SetLocation(vec2(10, (buttonIndex++) * 50.0f));
			button->SetSize(vec2(200, 40));
			button->SetOrigin(vec2(0, 40));
			button->SetAnchor(vec2(-1, -0.25f));
			return button;
		};



		UButton* login = makeButton();
		login->SetText("Login");
		login->SetCallback([this]() { GetGame()->SwitchLevel(LMainLevel::StaticClass()); });

		UButton* connect = makeButton();
		connect->SetText("Connect");
		connect->SetCallback(
			[this]()
			{
				GetGame()->GetEngine()->GetNetController()->JoinSession(GetGame()->GetEngine()->GetDefaultNetIdentity());
			}
		);

		UButton* host = makeButton();
		host->SetText("Host");
		host->SetCallback(
			[this]() 
			{ 
				if(GetGame()->GetEngine()->GetNetController()->HostSession(GetGame()->GetEngine()->GetDefaultNetIdentity()))
					GetGame()->SwitchLevel(LMainLevel::StaticClass());
			}
		);

		UButton* server = makeButton();
		server->SetText("Server List");

		UButton* settings = makeButton();
		settings->SetText("Settings");
		settings->SetDisabled(true);

		UButton* exit = makeButton();
		exit->SetText("Exit");
		exit->SetCallback([this]() { GetGame()->GetEngine()->Close(); });
	}

	// Left nav menu
	{
	
	}
}

