#pragma once
#include "Types.h"
#include <SFML\Graphics.hpp>


/**
* Hold's information about a specific keybinding
*/
struct KeyBinding 
{
private:
	friend class InputController;
	// The key to listen for 
	sf::Keyboard::Key m_key;
	// The mouse button to listen for 
	sf::Mouse::Button m_button;

	enum BindingMode
	{
		Keyboard, Mouse
	} m_bindingMode;
public:
	KeyBinding(const sf::Keyboard::Key& key) : m_key(key), m_button(sf::Mouse::Button::ButtonCount)
	{
		m_bindingMode = BindingMode::Keyboard;
	}
	KeyBinding(const sf::Mouse::Button& button) : m_key(sf::Keyboard::Unknown), m_button(button)
	{
		m_bindingMode = BindingMode::Mouse;
	}


	// Was this key pressed this update
	inline const bool& IsPressed() const { return bIsPressed; }

	// Was this key released this update
	inline const bool& IsReleased() const { return bIsReleased; }

	/// Is this key currently down
	inline const bool& IsHeld() const { return bIsHeld; }

private:
	friend class InputController;
	bool bIsPressed = false;
	bool bIsReleased = false;
	bool bIsHeld = false;
};



/**
* Centeral hub for all types of input
*/
class InputController
{
private:
	bool m_keyStates[sf::Keyboard::Key::KeyCount]{ false };
	bool m_mouseStates[sf::Mouse::Button::ButtonCount]{ false };
	ivec2 m_mousePosition;

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
	* Handles an SFML event
	* @param event			Information about the event
	*/
	void UpdateEvent(const sf::Event& event);


	/**
	* Getters & Setters
	*/
public:
	inline bool GetKeyState(const sf::Keyboard::Key& key) const { return m_keyStates[key]; }

	inline const ivec2& GetMouseLocation() const { return m_mousePosition; }
	inline bool GetMouseButtonState(const sf::Mouse::Button& key) const { return m_mouseStates[key]; }
};

