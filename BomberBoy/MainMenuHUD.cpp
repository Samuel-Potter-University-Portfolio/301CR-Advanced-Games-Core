#include "MainMenuHUD.h"


CLASS_SOURCE(AMainMenuHUD)


void AMainMenuHUD::OnBegin()
{
	Super::OnBegin();
	bIsTickable = true;

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


		m_loginButton = makeButton();
		m_loginButton->SetText("Login");
		m_loginButton->SetCallback([this]()
		{
			m_loginMenu.SetActive(true);
			m_hostMenu.SetActive(false);
			m_connectMenu.SetActive(false);
			m_serverMenu.SetActive(false);
			m_settingsMenu.SetActive(false);
		});


		m_connectButton = makeButton();
		m_connectButton->SetText("Connect");
		m_connectButton->SetCallback([this]()
		{
			m_loginMenu.SetActive(false);
			m_hostMenu.SetActive(false);
			m_connectMenu.SetActive(true);
			m_serverMenu.SetActive(false);
			m_settingsMenu.SetActive(false);
		});


		m_hostButton = makeButton();
		m_hostButton->SetText("Host");
		m_hostButton->SetCallback([this]()
		{
			m_loginMenu.SetActive(false);
			m_hostMenu.SetActive(true);
			m_connectMenu.SetActive(false);
			m_serverMenu.SetActive(false);
			m_settingsMenu.SetActive(false);
		});


		m_serverButton = makeButton();
		m_serverButton->SetText("Server List");
		m_serverButton->SetCallback([this]()
		{
			m_loginMenu.SetActive(false);
			m_hostMenu.SetActive(false);
			m_connectMenu.SetActive(false);
			m_serverMenu.SetActive(true);
			m_settingsMenu.SetActive(false);
		});


		m_settingsButton = makeButton();
		m_settingsButton->SetText("Settings");
		m_settingsButton->SetCallback([this]()
		{
			m_loginMenu.SetActive(false);
			m_hostMenu.SetActive(false);
			m_connectMenu.SetActive(false);
			m_serverMenu.SetActive(false);
			m_settingsMenu.SetActive(true);
		});


		m_exitButton = makeButton();
		m_exitButton->SetText("Exit");
		m_exitButton->SetCallback([this]() { GetGame()->GetEngine()->Close(); });
	}

	// Menus
	{
		const vec2 anchor(0, -0.15f);

		// Login menu
		m_loginMenu.SetActive(false);
		m_loginMenu.Build(this, defaultFont, defaultScaling, anchor);


		// Host menu
		m_hostMenu.SetActive(false);
		m_hostMenu.Build(this, defaultFont, defaultScaling, anchor);


		// Connect menu
		m_connectMenu.SetActive(false);
		m_connectMenu.Build(this, defaultFont, defaultScaling, anchor);


		m_serverMenu.SetActive(false);
		m_settingsMenu.SetActive(false);
	}


	// Setup which buttons should be disabled
#ifdef API_SUPPORTED
	//m_loginButton->SetDisabled(true);
	m_connectButton->SetDisabled(true);
	m_hostButton->SetDisabled(true);
	m_serverButton->SetDisabled(true);
	m_settingsButton->SetDisabled(true);
	//m_exitButton->SetDisabled(true);
#else
	m_loginButton->SetDisabled(true);
	//m_connectButton->SetDisabled(true);
	//m_hostButton->SetDisabled(true);
	m_serverButton->SetDisabled(true);
	m_settingsButton->SetDisabled(true);
	//m_exitButton->SetDisabled(true);
#endif
}

void AMainMenuHUD::OnTick(const float& deltaTime)
{
	Super::OnTick(deltaTime);

	// Setup which buttons should be disabled
#ifdef API_SUPPORTED
	OAPIController* controller = GetGame()->GetFirstObject<OAPIController>();

	if (controller != nullptr && controller->IsUserLoggedIn())
	{
		m_connectButton->SetDisabled(false);
		m_hostButton->SetDisabled(false);
		m_serverButton->SetDisabled(false);
	}
	else
	{
		m_connectButton->SetDisabled(true);
		m_hostButton->SetDisabled(true);
		m_serverButton->SetDisabled(true);
	}
#endif
}
