#include "HostMenu.h"

#include "MainLevel.h"


void HostMenu::Build(AHUD* hud, const sf::Font* font, const ULabel::ScalingMode& scalingMode, const vec2 anchor)
{
	m_identity = hud->GetGame()->GetEngine()->GetNetController()->GetLocalIdentity();
	const vec2 size(500, 300);


	SetupDefaultMenu(hud, "Host", font, scalingMode, size, anchor);
	

	m_launchButton = AddElement<UButton>(hud);
	m_launchButton->SetScalingMode(scalingMode);
	m_launchButton->SetFont(font);
	m_launchButton->SetText("Launch");
	m_launchButton->SetDisabled(true);

	m_launchButton->SetSize(vec2(200, 40));
	m_launchButton->SetOrigin(vec2(0, 0));
	m_launchButton->SetLocation(vec2(-240, 45));
	m_launchButton->SetAnchor(anchor);
	m_launchButton->SetCallback([this, hud]()
	{
		Game* game = hud->GetGame();

		const bool launched = game->GetNetController()->HostSession(m_identity,
			[](NetLayer* layer)
			{
				// TODO - Configure layer
			}
		);

		if (launched)
			game->SwitchLevel(LMainLevel::StaticClass());
	});


	m_ipField = AddElement<UInputField>(hud);
	m_ipField->SetScalingMode(scalingMode);
	m_ipField->SetFont(font);
	m_ipField->SetDefaultText("IP");
	m_ipField->SetText(hud->GetGame()->GetEngine()->GetNetController()->GetPublicIdentity().ip.toString());

	m_ipField->SetLocation(vec2(-240, -50));
	m_ipField->SetDefaultColour(Colour::White);
	m_ipField->SetAnchor(anchor);
	m_ipField->SetDisabled(true);


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
		try	{ port = (uint16)std::stoi(value); }
		catch (std::invalid_argument e) {}

		// Port must be greater than 1000
		if (port < 1000)
		{
			m_portField->SetText("");
			m_launchButton->SetDisabled(true);
		}
		else
		{
			m_portField->SetText(std::to_string(port)); // Set just incase value was over port limit
			m_identity.port = port;
			m_launchButton->SetDisabled(false);
		}
	});

}