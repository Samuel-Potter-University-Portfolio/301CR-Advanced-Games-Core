#include "Includes\Core\InputField.h"
#include "Includes\Core\Game.h"


CLASS_SOURCE(UInputField, CORE_API)
UInputField* UInputField::s_currentFocus = nullptr;


UInputField::UInputField() :
	m_defaultColour(210, 210, 210, 255),
	m_disabledColour(50, 50, 50, 255)
{
	bIsTickable = true;

	SetDrawBackground(true);
	SetPadding(2.4f);

	SetHorizontalAlignment(ULabel::HorizontalAlignment::Left);
	SetVerticalAlignment(ULabel::VerticalAlignment::Top);
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
		string text = GetText();
		string input = GetHUD()->GetInputController()->GetTypedString();
		if (!input.empty())
		{
			for (const char& c : input)
			{
				if (c == '\b')
				{
					if (text.size() != 0)
						text.erase(text.size() - 1);
				}
				else if (c == '\r')
				{
					s_currentFocus = nullptr;
					SetText(text);
					OnInputDefocus();
					return;
				}
				else
					text += c;
			}
		}

		SetText(text);
	}
}

void UInputField::OnMousePressed()
{
	if (IsDisabled())
		return;

	if (s_currentFocus != this && s_currentFocus != nullptr)
		s_currentFocus->OnInputDefocus();
	s_currentFocus = this;
	OnInputFocus();
}

void UInputField::OnInputDefocus() 
{
	if (!IsDisabled() && m_callback)
		m_callback(GetText());
}

void UInputField::OnDraw(sf::RenderWindow* window, const float& deltaTime) 
{
	// Change colour of button
	if (IsDisabled())
		SetColour(m_disabledColour);
	else
		SetColour(m_defaultColour);
	
	DrawDefaultRect(window);


	if (!IsFocused() && GetText().empty())
	{
		// Draw default text, if not active
		Colour colour = GetTextColour();
		colour.a = 150;
		DrawText(window, m_defaultText, colour, sf::Text::Italic);
	}
	else
	{
		// TODO - Cache clamped text
		DrawText(window, GetClampedText(GetText(), !IsFocused()), GetTextColour(), IsFocused() ? GetTextStyle() : sf::Text::Italic);
	}
}

string UInputField::GetClampedText(const string& msg, const bool& clampFront) const
{
	if (GetFont() == nullptr || msg.empty())
		return msg;


	sf::Text text;
	text.setFont(*GetFont());
	text.setCharacterSize(GetFontSize());
	// Leaving scale as default, will scale with window
	if (GetScalingMode() == ScalingMode::PixelPerfect)
		text.setScale(vec2(1, 1));



	// Make sure text doesn't overflow
	text.setString("..");
	const float maxWidth = GetSize().x - text.getLocalBounds().width * 2.0f - GetPadding();
	text.setString(msg);

	if (text.getLocalBounds().width > maxWidth)
	{
		// Clamp from back
		if (!clampFront)
		{
			string clamped = "";
			for (uint32 i = msg.size() - 1; i > 0; --i)
			{
				text.setString(".." + msg[i] + clamped);
				if (text.getLocalBounds().width >= maxWidth)
					break;
				else
					clamped = msg[i] + clamped;
			}

			return ".." + clamped;
		}

		// Clamp from front
		else
		{
			string clamped = "";
			text.setString("");
			for (const char& c : msg)
			{
				text.setString(clamped + c + "..");
				if (text.getLocalBounds().width >= maxWidth)
					break;
				else
					clamped += c;
			}
			return clamped + "..";
		}

	}
	return msg;
}