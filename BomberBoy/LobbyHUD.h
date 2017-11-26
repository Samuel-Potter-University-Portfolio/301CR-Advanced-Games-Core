#pragma once
#include "Core\Core-Common.h"
#include "MenuContainer.h"


#include "BPlayerController.h"


/**
* Reprensents a single player entry
*/
class PlayerTag : public MenuContainer
{
private:
	friend class ALobbyHUD;
	OBPlayerController* m_player = nullptr;
	const AnimationSheet* m_animation;
	ULabel* m_background;
	ULabel* m_icon;
	ULabel* m_name;

public:
	/**
	* Builds the host menu
	* @param hud			The hud to store elements under
	* @param font			The font to use for the title
	* @param scalingMode	The method of scaling to use
	* @param index			The index of the tag
	*/
	void Build(AHUD* hud, const sf::Font* font, const ULabel::ScalingMode& scalingMode, const uint32& index);

	/**
	* Update the look of this tag based on the player
	*/
	void SetPlayer(OBPlayerController* player);

	/**
	* Set the style of this button to be locked
	*/
	void SetLockedStyle();
};



/**
* HUD for displaying connected players and map vote
*/
class ALobbyHUD : public AHUD
{
	CLASS_BODY()
private:
	OBPlayerController* m_localPlayer = nullptr;
	MenuContainer m_voteMenu;
	PlayerTag m_playerTags[16];

public:
	ALobbyHUD();
	virtual void OnBegin() override;

	void OnPlayerConnect(OBPlayerController* player);
	void OnPlayerDisconnect(OBPlayerController* player);
};
