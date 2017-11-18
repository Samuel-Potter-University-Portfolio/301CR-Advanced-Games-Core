#pragma once
#include <SFML\Graphics.hpp>


/**
* Hold's information about a specific keybinding
*/
struct KeyBinding 
{
public:
	// The key to listen for 
	sf::Keyboard::Key key;	

	KeyBinding(const sf::Keyboard::Key& key = sf::Keyboard::Unknown) : key(key)
	{
		bIsPressed = false;
		bIsReleased = false;
		bIsHeld = false;
	}


	// Was this key pressed this update
	inline const bool& IsPressed() const { return bIsPressed; }

	// Was this key released this update
	inline const bool& IsReleased() const { return bIsReleased; }

	/// Is this key currently down
	inline const bool& IsHeld() const { return bIsHeld; }

private:
	friend class InputController;
	bool bIsPressed;
	bool bIsReleased;
	bool bIsHeld;
};



/**
* Centeral hub for all types of input
*/
class InputController
{
private:
	bool m_keyStates[sf::Keyboard::Key::KeyCount]{ false };

public:
	InputController();
	~InputController();

	/**
	* Callback from engine for every tick by main
	* @param game			The game that is currently active
	* @param deltaTime		Time since last update (In seconds)
	*/
	void HandleUpdate(class Game* game, const float& deltaTime);

	/**
	* Internally updates the state of this key 
	* @param event			Information about the key event
	* @param pressed		Was this a keypressed event (Or a released, if false)
	*/
	void UpdateKeystate(const sf::Event::KeyEvent& event, const bool& pressed);
};

