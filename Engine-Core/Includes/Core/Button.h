#pragma once
#include "GUIBase.h"
#include <functional>


typedef std::function<void()> ButtonCallback;


/**
* A button which will call a specific callback when pressed
*/
class CORE_API UButton : public UGUIBase
{
	CLASS_BODY()
private:
	ButtonCallback m_callback;
	bool bPressedOnThis = false;

	Colour m_defaultColour;
	Colour m_enteredColour;
	Colour m_pressedColour;
	Colour m_disabledColour;


	string m_text;
	Colour m_textColour;

	const sf::Font* m_font = nullptr;
	uint32 m_fontSize = 24;

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
	void OnDraw(sf::RenderWindow* window, const float& deltaTime) override;
	/**
	* Draw the default text using applied settings
	* @param window			The window to draw to
	*/
	void DrawDefaultButtonText(sf::RenderWindow* window);

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


	inline void SetText(const string& value) { m_text = value; }
	inline const string& GetText() const { return m_text; }

	inline void SetTextColour(const Colour& value) { m_textColour = value; }
	inline const Colour& GetTextColour() const { return m_textColour; }

	inline void SetFont(const sf::Font* value) { m_font = value; }
	inline const sf::Font* GetFont() const { return m_font; }

	inline void SetFontSize(const uint32& value) { m_fontSize = value; }
	inline const uint32& GetFontSize() const { return m_fontSize; }
};

