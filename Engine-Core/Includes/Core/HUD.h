#pragma once
#include "Actor.h"
#include "GUIBase.h"

/**
* Contains all GUI elements and only exists on clients
*/
class CORE_API AHUD : public AActor
{
	CLASS_BODY()
private:
	std::vector<UGUIBase*> m_elements;

public:
	AHUD();
	virtual ~AHUD();


	/**
	* Called after all actors have been drawn
	* @param window			The window to draw to
	* @param deltaTime		Time since last draw in seconds)
	*/
	void DisplayUpdate(sf::RenderWindow* window, const float& deltaTime);


	/**
	* Add a GUI element to the HUD
	* @param type			The GUI type to add
	* @returns The new object (Or nullptr if failed)
	*/
	UGUIBase* AddElement(SubClassOf<UGUIBase> type);

	/**
	* Getters & Setters
	*/
public:
};

