#pragma once
#pragma once
#include "MenuContainer.h"


class ConnectMenu : public MenuContainer
{
private:
	NetIdentity	m_identity;

	UInputField* m_ipField = nullptr;
	UInputField* m_portField = nullptr;
	UButton*	 m_joinButton = nullptr;

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
	/** Callback for when desired identity changes */
	void OnIdentityChange();
};

