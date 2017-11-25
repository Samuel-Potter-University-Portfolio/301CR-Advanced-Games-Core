#pragma once
#include "Label.h"
#include <functional>


typedef std::function<void()> ButtonCallback;


/**
* A button which will call a specific callback when pressed
*/
class CORE_API UButton : public ULabel
{
	CLASS_BODY()
private:
	ButtonCallback m_callback;
	bool bPressedOnThis = false;

	Colour m_defaultColour;
	Colour m_enteredColour;
	Colour m_pressedColour;
	Colour m_disabledColour;

public:
	UButton();

	virtual void OnMouseExit() override;

	virtual void OnMousePressed() override;
	virtual void OnMouseReleased() override;

	/**
	* Called when this GUI should be drawn to the screen
	* @param window			The window to draw to
	* @param deltaTime		Time since last draw in seconds
	*/
	virtual void OnDraw(sf::RenderWindow* window, const float& deltaTime) override;

	/**
	* Getters & Setters
	*/
public:
	inline void SetCallback(ButtonCallback callback) { m_callback = callback; }


	inline void SetDefaultColour(const Colour& value) { m_defaultColour = value; }
	inline const Colour& GetDefaultColour() const { return m_defaultColour; }

	inline void SetEnteredColour(const Colour& value) { m_enteredColour = value; }
	inline const Colour& GetEnteredColour() const { return m_enteredColour; }

	inline void SetPressedColour(const Colour& value) { m_pressedColour = value; }
	inline const Colour& GetPressedColour() const { return m_pressedColour; }

	inline void SetDisabledColour(const Colour& value) { m_disabledColour = value; }
	inline const Colour& GetDisabledColour() const { return m_disabledColour; }
};

