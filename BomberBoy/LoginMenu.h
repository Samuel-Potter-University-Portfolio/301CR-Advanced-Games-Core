#pragma once
#include "MenuContainer.h"
#include "APIController.h"


class LoginMenu : public MenuContainer
{
private:
	OAPIController*	m_apiController = nullptr;

	UInputField*	m_accountField = nullptr;
	UInputField*	m_passwordField = nullptr;
	ULabel*			m_loginShield = nullptr;
	ULabel*			m_loginStatus = nullptr;
	UButton*		m_loginButton = nullptr;

public:
	/**
	* Builds the host menu
	* @param hud			The hud to store elements under
	* @param font			The font to use for the title
	* @param scalingMode	The method of scaling to use
	* @param anchor			The anchor to attatch the menu to
	*/
	void Build(AHUD* hud, const sf::Font* font, const ULabel::ScalingMode& scalingMode, const vec2 anchor);

private:
	/** Callback for when the login fields are changed */
	void OnFieldsChange();
};

