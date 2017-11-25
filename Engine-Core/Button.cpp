#include "Includes\Core\Button.h"


CLASS_SOURCE(UButton, CORE_API)


UButton::UButton() :
	m_defaultColour(210, 210, 210, 255),
	m_enteredColour(225, 225, 225, 255),
	m_pressedColour(255, 255, 255, 255), 
	m_disabledColour(50, 50, 50, 255),

	m_text("Button"),
	m_textColour(0, 0, 0, 255)
{
	SetColour(m_defaultColour);
}


void UButton::OnMouseExit()
{
	bPressedOnThis = false;
}

void UButton::OnMousePressed()
{
	bPressedOnThis = true;
}

void UButton::OnMouseReleased()
{
	if (bPressedOnThis && !IsDisabled() && m_callback)
		m_callback();
}

void UButton::OnDraw(sf::RenderWindow* window, const float& deltaTime) 
{
	// Change colour of button
	if (IsDisabled())
		SetColour(m_disabledColour);

	else if (IsMouseOver())
	{
		if(bPressedOnThis)
			SetColour(m_pressedColour);
		else
			SetColour(m_enteredColour);
	}
	else
		SetColour(m_defaultColour);

	DrawDefaultRect(window);
	DrawDefaultButtonText(window);
}

void UButton::DrawDefaultButtonText(sf::RenderWindow* window) 
{
	// SFML won't render it anyway, so just exit early
	if (m_font == nullptr)
		return;


	sf::Text text;
	text.setString(m_text);
	text.setOrigin(GetOrigin());
	text.setPosition(GetLocation() + GetSize() * 0.5f);

	text.setFont(*m_font);
	text.setFillColor(m_textColour);

	if (IsDisabled())
	{
		text.setFillColor(Colour(m_textColour.r, m_textColour.g, m_textColour.b, 150));
		text.setStyle(sf::Text::Italic);
	}

	text.setCharacterSize(m_fontSize);
	// Leaving scale as default, will scale with window
	if (GetScalingMode() == ScalingMode::PixelPerfect)
		text.setScale(vec2(1, 1));


	// Move to get text in centre of box
	sf::FloatRect bounds = text.getGlobalBounds();
	text.setPosition(text.getPosition() - vec2(bounds.width * 0.5f, bounds.height));

	window->draw(text);
}