#include "LoginMenu.h"
#include <Windows.h>


void LoginMenu::Build(AHUD* hud, const sf::Font* font, const ULabel::ScalingMode& scalingMode, const vec2 anchor)
{
	m_apiController = hud->GetGame()->GetFirstObject<OAPIController>();
	// Cannot function without having access to the api controller
	if (m_apiController == nullptr)
		return;

	const vec2 size(500, 300);
	SetupDefaultMenu(hud, "Login", font, scalingMode, size, anchor);


	m_loginStatus = AddElement<ULabel>(hud);
	m_loginStatus->SetScalingMode(scalingMode);
	m_loginStatus->SetFont(font);
	m_loginStatus->SetText("Not logged in");
	m_loginStatus->SetVerticalAlignment(ULabel::VerticalAlignment::Top);
	m_loginStatus->SetHorizontalAlignment(ULabel::HorizontalAlignment::Left);

	m_loginStatus->SetSize(vec2(100, 100));
	m_loginStatus->SetOrigin(vec2(0, 0));
	m_loginStatus->SetAnchor(anchor);
	m_loginStatus->SetLocation(vec2(-240, 30));


	m_loginButton = AddElement<UButton>(hud);
	m_loginButton->SetScalingMode(scalingMode);
	m_loginButton->SetVerticalAlignment(ULabel::VerticalAlignment::Top);
	m_loginButton->SetFont(font);
	m_loginButton->SetText("Login");
	m_loginButton->SetDisabled(true);

	m_loginButton->SetSize(vec2(200, 40));
	m_loginButton->SetOrigin(vec2(0, 0));
	m_loginButton->SetLocation(vec2(40, 135));
	m_loginButton->SetAnchor(anchor);
	m_loginButton->SetCallback([this]()
	{
		// Logout user
		if (m_apiController->IsUserLoggedIn())
		{
			m_apiController->LogoutUser();
			m_accountField->SetDisabled(false);
			m_passwordField->SetDisabled(false);
			m_loginShield->SetVisible(false);

			m_loginStatus->SetText("Not logged in");
			m_loginButton->SetText("Login");
		}

		// Login user
		else
		{
			m_loginShield->SetVisible(true);
			m_apiController->LoginUser(m_accountField->GetText(), m_passwordField->GetText(),
			[this](Http::Response& response)
			{
				m_loginShield->SetVisible(false);

				if (response.getStatus() == 200)
				{
					m_accountField->SetDisabled(true);
					m_passwordField->SetDisabled(true);

					m_accountField->SetText("");
					m_passwordField->SetText("");

					m_loginStatus->SetText("Logged in as\n" + m_apiController->GetUserDisplayName());
					m_loginButton->SetText("Logout");
				}
				else
				{
					m_loginStatus->SetText("Login failed..");
				}
			});
		}
	});


	UButton* registerButton = AddElement<UButton>(hud);
	registerButton->SetScalingMode(scalingMode);
	registerButton->SetVerticalAlignment(ULabel::VerticalAlignment::Top);
	registerButton->SetFont(font);
	registerButton->SetText("Register");
	registerButton->SetTextColour(Colour::Blue);
	registerButton->SetTextStyle(sf::Text::Underlined | sf::Text::Italic);

	registerButton->SetSize(vec2(200, 40));
	registerButton->SetOrigin(vec2(0, 0));
	registerButton->SetLocation(vec2(40, 135 - 50));
	registerButton->SetAnchor(anchor);
	registerButton->SetCallback([this]()
	{
		// Open register page
		ShellExecute(nullptr, "open", (m_apiController->GetDomain() + "/register.html").c_str(), nullptr, nullptr, SW_SHOWNORMAL);
	});


	m_accountField = AddElement<UInputField>(hud);
	m_accountField->SetScalingMode(scalingMode);
	m_accountField->SetFont(font);
	m_accountField->SetDefaultText("Account email");

	m_accountField->SetLocation(vec2(-240, -50));
	m_accountField->SetDefaultColour(Colour::White);
	m_accountField->SetAnchor(anchor);
	m_accountField->SetCallback([this](string) { OnFieldsChange(); });


	m_passwordField = AddElement<UInputField>(hud);
	m_passwordField->SetScalingMode(scalingMode);
	m_passwordField->SetFont(font);
	m_passwordField->SetDefaultText("Password");
	m_passwordField->SetSensitiveText(true);

	m_passwordField->SetLocation(vec2(-240, -5));
	m_passwordField->SetDefaultColour(Colour::White);
	m_passwordField->SetAnchor(anchor);
	m_passwordField->SetCallback([this](string) { OnFieldsChange(); });


	m_loginShield = AddElement<ULabel>(hud);
	m_loginShield->SetScalingMode(ULabel::ScalingMode::Expand);
	m_loginShield->SetColour(Colour(0, 0, 0, 100));
	m_loginShield->SetDrawBackground(true);
	m_loginShield->SetVisible(false);

	m_loginShield->SetSize(vec2(100000, 100000));
	m_loginShield->SetAnchor(vec2(0, 0));
	m_loginShield->SetOrigin(m_loginShield->GetSize() * 0.5f);
}

void LoginMenu::OnFieldsChange() 
{
	if (m_accountField->GetText().size() != 0 && m_passwordField->GetText().size() != 0)
		m_loginButton->SetDisabled(false);
	else
		m_loginButton->SetDisabled(true);
}