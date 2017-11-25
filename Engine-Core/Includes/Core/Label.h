#pragma once
#include "GUIBase.h"


class CORE_API ULabel : public UGUIBase
{
	CLASS_BODY()
public:
	enum HorizontalAlignment
	{
		Centre,
		Left,
		Right
	};
	enum VerticalAlignment
	{
		Middle,
		Top,
		Bottom
	};

private:
	string m_text;
	Colour m_textColour;

	const sf::Font* m_font = nullptr;
	uint32 m_fontSize = 24;
	uint32 m_style = sf::Text::Regular;
	float m_padding = 5.0f;
	HorizontalAlignment m_horiAlignment = HorizontalAlignment::Centre;
	VerticalAlignment m_vertAlignment = VerticalAlignment::Middle;

protected:
	bool bDrawBackground = false;

public:
	ULabel();

	/**
	* Called when this GUI should be drawn to the screen
	* @param window			The window to draw to
	* @param deltaTime		Time since last draw in seconds
	*/
	virtual void OnDraw(sf::RenderWindow* window, const float& deltaTime) override;
	/**
	* Draw the default text using applied settings
	* @param window			The window to draw to
	*/
	void DrawDefaultText(sf::RenderWindow* window);
	/**
	* Draw text using applied settings
	* @param window			The window to draw to
	* @param text			The text to draw
	* @param sytle			The style to use when drawing
	*/
	void DrawText(sf::RenderWindow* window, const string& text, const Colour& colour, const uint32& style);


	/**
	* Getters & Setters
	*/
public:
	inline void SetText(const string& value) { m_text = value; }
	inline const string& GetText() const { return m_text; }

	inline void SetTextColour(const Colour& value) { m_textColour = value; }
	inline const Colour& GetTextColour() const { return m_textColour; }

	inline void SetFont(const sf::Font* value) { m_font = value; }
	inline const sf::Font* GetFont() const { return m_font; }

	inline void SetFontSize(const uint32& value) { m_fontSize = value; }
	inline const uint32& GetFontSize() const { return m_fontSize; }

	inline void SetTextStyle(const uint32& value) { m_style = value; }
	inline const uint32& GetTextStyle() const { return m_style; }

	inline void SetPadding(const float& value) { m_padding = value; }
	inline const float& GetPadding() const { return m_padding; }

	inline void SetHorizontalAlignment(const HorizontalAlignment& value) { m_horiAlignment = value; }
	inline const HorizontalAlignment& GetHorizontalAlignment() const { return m_horiAlignment; }
	inline void SetVerticalAlignment(const VerticalAlignment& value) { m_vertAlignment = value; }
	inline const VerticalAlignment& GetVerticalAlignment() const { return m_vertAlignment; }

	inline void SetDrawBackground(const bool& value) { bDrawBackground = value; }
	inline const bool& DoesDrawBackground() const { return bDrawBackground; }
};

