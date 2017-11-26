#include "MainMenuHUD.h"
#include "MainLevel.h"


CLASS_SOURCE(AMainMenuHUD)


#include "ChatWidget.h"

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
		// Use lambda for placing buttons, to make sure they are all in the correct place
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
		login->SetCallback([this]() 
		{
			m_loginMenu.SetActive(true);
			m_hostMenu.SetActive(false);
			m_connectMenu.SetActive(false);
			m_serverMenu.SetActive(false);
			m_settingsMenu.SetActive(false);
		});
		login->SetDisabled(true);


		UButton* connect = makeButton();
		connect->SetText("Connect");
		connect->SetCallback([this]()
		{
			m_loginMenu.SetActive(false);
			m_hostMenu.SetActive(false);
			m_connectMenu.SetActive(true);
			m_serverMenu.SetActive(false);
			m_settingsMenu.SetActive(false);
		});


		UButton* host = makeButton();
		host->SetText("Host");
		host->SetCallback([this]()
		{
			m_loginMenu.SetActive(false);
			m_hostMenu.SetActive(true);
			m_connectMenu.SetActive(false);
			m_serverMenu.SetActive(false);
			m_settingsMenu.SetActive(false);
		});


		UButton* server = makeButton();
		server->SetText("Server List");
		server->SetCallback([this]()
		{
			m_loginMenu.SetActive(false);
			m_hostMenu.SetActive(false);
			m_connectMenu.SetActive(false);
			m_serverMenu.SetActive(true);
			m_settingsMenu.SetActive(false);
		});
		server->SetDisabled(true);


		UButton* settings = makeButton();
		settings->SetText("Settings");
		settings->SetCallback([this]()
		{
			m_loginMenu.SetActive(false);
			m_hostMenu.SetActive(false);
			m_connectMenu.SetActive(false);
			m_serverMenu.SetActive(false);
			m_settingsMenu.SetActive(true);
		});
		settings->SetDisabled(true);


		UButton* exit = makeButton();
		exit->SetText("Exit");
		exit->SetCallback([this]() { GetGame()->GetEngine()->Close(); });
	}

	// Menus
	{
		const vec2 anchor(0, -0.15f);

		// Login menu
		m_loginMenu.SetActive(false);


		// Host menu
		m_hostMenu.SetActive(false);
		m_hostMenu.Build(this, defaultFont, defaultScaling, anchor);


		// Connect menu
		m_connectMenu.SetActive(false);
		m_connectMenu.Build(this, defaultFont, defaultScaling, anchor);


		m_serverMenu.SetActive(false);
		m_settingsMenu.SetActive(false);
	}


	UChatWidget* chat = AddElement<UChatWidget>();
	chat->SetFont(defaultFont);
}

