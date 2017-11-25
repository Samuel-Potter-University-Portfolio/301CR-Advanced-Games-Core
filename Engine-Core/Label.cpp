#include "Includes\Core\Label.h"


CLASS_SOURCE(ULabel, CORE_API)


ULabel::ULabel() :
	m_textColour(0, 0, 0, 255)
{
}

void ULabel::OnDraw(sf::RenderWindow* window, const float& deltaTime)
{
	if (bDrawBackground)
		DrawDefaultRect(window);
	DrawDefaultText(window);
}

void ULabel::DrawText(sf::RenderWindow* window, const string& msg, const Colour& colour, const uint32& style)
{
	// SFML won't render it anyway, so just exit early
	if (m_font == nullptr || msg.empty())
		return;

	sf::Text text;
	text.setString(msg);
	text.setOrigin(GetOrigin());
	text.setPosition(GetLocation());

	text.setFont(*m_font);
	text.setStyle(style);
	text.setFillColor(colour);


	text.setCharacterSize(m_fontSize);
	// Leaving scale as default, will scale with window
	if (GetScalingMode() == ScalingMode::PixelPerfect)
		text.setScale(vec2(1, 1));



	sf::FloatRect bounds = text.getLocalBounds();
	float vertAlign = 0;

	if (m_vertAlignment == VerticalAlignment::Top)
		vertAlign = m_padding;
	else if (m_vertAlignment == VerticalAlignment::Middle)
		vertAlign = GetSize().y * 0.5f - bounds.height;
	else if (m_vertAlignment == VerticalAlignment::Bottom)
		vertAlign = GetSize().y - bounds.height * 2.0f - m_padding;


	if (m_horiAlignment == HorizontalAlignment::Left)
		text.setPosition(GetLocation() + vec2(m_padding, vertAlign));

	else if (m_horiAlignment == HorizontalAlignment::Centre)
		text.setPosition(GetLocation() + vec2((GetSize().x - bounds.width) * 0.5f, vertAlign));

	else if (m_horiAlignment == HorizontalAlignment::Right)
		text.setPosition(GetLocation() + vec2(GetSize().x - bounds.width - m_padding, vertAlign));

	window->draw(text);
}

void ULabel::DrawDefaultText(sf::RenderWindow* window)
{
	DrawText(window, m_text, m_textColour, m_style);
}