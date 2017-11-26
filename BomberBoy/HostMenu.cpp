#include "HostMenu.h"

#include "MainLevel.h"


void HostMenu::Build(AHUD* hud, const sf::Font* font, const ULabel::ScalingMode& scalingMode, const vec2 anchor)
{
	m_identity.ip = "localhost";
	const vec2 size(500, 300);


	SetupDefaultMenu(hud, "Host", font, scalingMode, size, anchor);
	

	m_launchButton = AddElement<UButton>(hud);
	m_launchButton->SetScalingMode(scalingMode);
	m_launchButton->SetFont(font);
	m_launchButton->SetText("Launch");
	m_launchButton->SetDisabled(true);

	m_launchButton->SetSize(vec2(200, 40));
	m_launchButton->SetOrigin(vec2(0, 0));
	m_launchButton->SetLocation(vec2(40, 135));
	m_launchButton->SetAnchor(anchor);
	m_launchButton->SetCallback([this, hud]()
	{
		Game* game = hud->GetGame();

		const bool launched = game->GetNetController()->HostSession(m_identity,
			[this](NetLayer* layer)
			{
				layer->GetSession()->SetMaxPlayerCount(playerCount);

				DefaultNetLayer* dlayer = dynamic_cast<DefaultNetLayer*>(layer);
				if (dlayer != nullptr)
				{
					dlayer->SetPassword(m_pswrdField->GetText());
					LOG("Using password..");
				}
			}
		);

		if (launched)
			game->SwitchLevel(LMainLevel::StaticClass());
	});


	m_ipField = AddElement<UInputField>(hud);
	m_ipField->SetScalingMode(scalingMode);
	m_ipField->SetFont(font);
	m_ipField->SetDefaultText("IP");

	m_ipField->SetLocation(vec2(-240, -50));
	m_ipField->SetDefaultColour(Colour::White);
	m_ipField->SetAnchor(anchor);
	m_ipField->SetText(m_identity.ip.toString());
	m_ipField->SetCallback([this](string value)
	{
		m_identity.ip = value;
		if (m_identity.ip.toInteger() == 0)
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


	m_playerCountField = AddElement<UInputField>(hud);
	m_playerCountField->SetScalingMode(scalingMode);
	m_playerCountField->SetFont(font);
	m_playerCountField->SetDefaultText("Max Players");

	m_playerCountField->SetLocation(vec2(40, -5));
	m_playerCountField->SetDefaultColour(Colour::White);
	m_playerCountField->SetAnchor(anchor);
	m_playerCountField->SetCallback([this](string value)
	{
		int32 count = 0;
		try { count = std::stoi(value); }
		catch (std::invalid_argument e) {}

		// Count cannot be more than 16
		playerCount = (uint16)std::min(16, std::max(2, count));
		m_playerCountField->SetText(std::to_string(playerCount));
	});
}

void HostMenu::OnIdentityChange()
{
	m_launchButton->SetDisabled(m_identity.ip.toInteger() == 0 || m_identity.port < 1000);
}