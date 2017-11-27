#pragma once
#include "MenuContainer.h"
#include "LobbyController.h"
#include "BPlayerController.h"

/**
* Map vote menu used in lobby screen
*/
class MapVoteMenu : public MenuContainer
{
private:
	ALobbyController* m_levelController;
	OBPlayerController* m_playerController;

	std::vector<ULabel*> m_levelVoteText;
	ULabel* m_timerText;

public:
	/**
	* Builds the host menu
	* @param hud			The hud to store elements under
	* @param font			The font to use for the title
	* @param scalingMode	The method of scaling to use
	* @param anchor			The anchor to attatch the menu to
	*/
	void Build(AHUD* hud, const sf::Font* font, const ULabel::ScalingMode& scalingMode, const vec2 anchor);

	/**
	* Updates the display of this card
	* @param hud			The hud this is under
	*/
	void UpdateDisplay(AHUD* hud);
};

