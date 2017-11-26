#include "ConnectMenu.h"


void ConnectMenu::Build(AHUD* hud, const sf::Font* font, const ULabel::ScalingMode& scalingMode, const vec2 anchor)
{
	const vec2 size(500, 300);


	SetupDefaultMenu(hud, "Connect", font, scalingMode, size, anchor);


	m_joinButton = AddElement<UButton>(hud);
	m_joinButton->SetScalingMode(scalingMode);
	m_joinButton->SetFont(font);
	m_joinButton->SetText("Connect");
	m_joinButton->SetDisabled(true);

	m_joinButton->SetSize(vec2(200, 40));
	m_joinButton->SetOrigin(vec2(0, 0));
	m_joinButton->SetLocation(vec2(40, 135));
	m_joinButton->SetAnchor(anchor);
	m_joinButton->SetCallback([this, hud]()
	{
		Game* game = hud->GetGame();

		const bool launched = game->GetNetController()->JoinSession(m_identity,
			[this](NetLayer* layer)
			{
				DefaultNetLayer* dlayer = dynamic_cast<DefaultNetLayer*>(layer);
				if (dlayer != nullptr)
				{
					dlayer->SetPassword(m_pswrdField->GetText());
				}
			}
		);

		//if (launched)
		//	game->SwitchLevel(LMainLevel::StaticClass());
	});


	m_ipField = AddElement<UInputField>(hud);
	m_ipField->SetScalingMode(scalingMode);
	m_ipField->SetFont(font);
	m_ipField->SetDefaultText("IP");

	m_ipField->SetLocation(vec2(-240, -50));
	m_ipField->SetDefaultColour(Colour::White);
	m_ipField->SetAnchor(anchor);
	m_ipField->SetCallback([this](string value)
	{
		m_identity.ip = value;
		if(m_identity.ip.toInteger() == 0)
			m_ipField->SetText("");

		OnIdentityChange();
	});


	m_portField = AddElement<UInputField>(hud);
	m_portField->SetScalingMode(scalingMode);
	m_portField->SetFont(font);
	m_portField->SetDefaultText("Port");

	m_portField->SetLocation(vec2(-240, -5));
	m_portField->SetDefaultColour(Colour::White);
	m_portField->SetAnchor(anchor);
	m_portField->SetCallback([this](string value)
	{
		uint16 port = 0;
		try { port = (uint16)std::stoi(value); }
		catch (std::invalid_argument e) {}

		// Port must be greater than 1000
		if (port < 1000)
		{
			m_portField->SetText("");
			m_identity.port = 0;
			OnIdentityChange();
		}
		else
		{
			m_portField->SetText(std::to_string(port)); // Set just incase value was over port limit
			m_identity.port = port;
			OnIdentityChange();
		}
	});


	m_pswrdField = AddElement<UInputField>(hud);
	m_pswrdField->SetScalingMode(scalingMode);
	m_pswrdField->SetFont(font);
	m_pswrdField->SetDefaultText("Password");
	m_pswrdField->SetSensitiveText(true);

	m_pswrdField->SetLocation(vec2(40, -50));
	m_pswrdField->SetDefaultColour(Colour::White);
	m_pswrdField->SetAnchor(anchor);
}

void ConnectMenu::OnIdentityChange() 
{
	m_joinButton->SetDisabled(m_identity.ip.toInteger() == 0 || m_identity.port < 1000);
}