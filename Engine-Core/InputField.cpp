#include "Includes\Core\InputField.h"
#include "Includes\Core\Game.h"


CLASS_SOURCE(UInputField, CORE_API)
UInputField* UInputField::s_currentFocus = nullptr;


UInputField::UInputField() :
	m_textColour(0,0,0,255)
{
	bIsTickable = true;
}

void UInputField::OnTick(const float& deltaTime) 
{
	Super::OnTick(deltaTime);

	if (IsFocused())
	{
		// Unfocus
		if (!IsMouseOver() && GetHUD()->GetMouse()->HasAnyInput())
		{
			s_currentFocus = nullptr;
			OnInputDefocus();
			return;
		}

		// Handle input
		string input = GetHUD()->GetInputController()->GetTypedString();
		if (!input.empty())
		{
			for (const char& c : input)
			{
				if (c == '\b')
				{
					if (m_value.size() != 0)
						m_value.erase(m_value.size() - 1);
				}
				else if (c == '\r')
				{
					s_currentFocus = nullptr;
					OnInputDefocus();
					return;
				}
				else
					m_value += c;
			}
		}
	}
}

void UInputField::OnMousePressed()
{
	s_currentFocus = this;
	OnInputFocus();
}


void UInputField::OnDraw(sf::RenderWindow* window, const float& deltaTime) 
{
	DrawDefaultRect(window);
	DrawDefaultText(window);
}

void UInputField::DrawDefaultText(sf::RenderWindow* window) 
{
	const float padding = 5.0f;

	// SFML won't render it anyway, so just exit early
	if (m_font == nullptr)
		return;


	sf::Text text; 
	text.setOrigin(GetOrigin());
	text.setPosition(GetLocation() + vec2(padding, padding));

	text.setFont(*m_font);
	text.setFillColor(IsFocused() ? m_textColour : Colour(m_textColour.r, m_textColour.g, m_textColour.b, 150));
	text.setStyle(IsFocused() ? sf::Text::Regular : sf::Text::Italic);

	text.setCharacterSize(m_fontSize);
	// Leaving scale as default, will scale with window
	if (GetScalingMode() == ScalingMode::PixelPerfect)
		text.setScale(vec2(1, 1));



	// Just display place holder text
	if (!IsFocused() && m_value.empty())
	{
		text.setString(m_defaultValue);
	}
	else
	{
		// Make sure text doesn't overflow
		text.setString(m_value);
		const float maxWidth = GetSize().x - 30.0f;

		if (text.getLocalBounds().width > maxWidth)
		{
			string value = "";
			for (uint32 i = m_value.size() - 1; i > 0; --i)
			{
				text.setString(".." + m_value[i] + value);
				if (text.getLocalBounds().width >= maxWidth)
				{
					text.setString(".." + value);
					break;
				}
				else
					value = m_value[i] + value;
			}
		}

	}

	window->draw(text);
}