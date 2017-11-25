#pragma once
#include "GUIBase.h"

/**
* Represents a text input field that the user may focus on and type in
*/
class CORE_API UInputField : public UGUIBase
{
	CLASS_BODY()
private:
	static UInputField* s_currentFocus;

	string m_value;
	string m_defaultValue = "Input";
	Colour m_textColour;

	const sf::Font* m_font = nullptr;
	uint32 m_fontSize = 24;

public:
	UInputField();


	virtual void OnTick(const float& deltaTime) override;
	virtual void OnMousePressed() override;

	/**
	* Callback for when the user focuses on this field
	*/
	virtual void OnInputFocus() {}
	/**
	* Callback for when the user defocusses on this field
	* May be called for user pressing return or clicking off of the field
	*/
	virtual void OnInputDefocus() {}

	/**
	* Called when this GUI should be drawn to the screen
	* @param window			The window to draw to
	* @param deltaTime		Time since last draw in seconds)
	*/
	void OnDraw(sf::RenderWindow* window, const float& deltaTime) override;
	/**
	* Draw the default text using applied settings
	* @param window			The window to draw to
	*/
	void DrawDefaultText(sf::RenderWindow* window);


	/**
	* Getters & Setters
	*/
public:
	inline bool IsFocused() const { return s_currentFocus == this; }

	inline void SetValue(const string& value) { m_value = value; }
	inline const string& GetValue() const { return m_value; }

	inline void SetDefaultValue(const string& value) { m_defaultValue = value; }
	inline const string& GetDefaultValue() const { return m_defaultValue; }

	inline void SetTextColour(const Colour& value) { m_textColour = value; }
	inline const Colour& GetTextColour() const { return m_textColour; }

	inline void SetFont(const sf::Font* value) { m_font = value; }
	inline const sf::Font* GetFont() const { return m_font; }

	inline void SetFontSize(const uint32& value) { m_fontSize = value; }
	inline const uint32& GetFontSize() const { return m_fontSize; }
};

