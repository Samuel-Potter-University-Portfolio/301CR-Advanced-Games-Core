#include "Includes\Core\InputField.h"
#include "Includes\Core\Game.h"


CLASS_SOURCE(UInputField, CORE_API)
UInputField* UInputField::s_currentFocus = nullptr;


UInputField::UInputField() :
	m_defaultColour(210, 210, 210, 255),
	m_disabledColour(50, 50, 50, 255)
{
	bIsTickable = true;

	SetOrigin(vec2(0, 0));
	SetSize(vec2(200, 35));
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
					SetText(text);
					OnInputDefocus();
					s_currentFocus = nullptr;
					return;
				}
				else
					text += c;
			}

			SetText(text);
			OnType();
		}
	}
}

void UInputField::OnMousePressed()
{
	if (IsDisabled())
		return;

	if (s_currentFocus != this && s_currentFocus != nullptr)
	{
		UInputField* other = s_currentFocus;
		s_currentFocus = nullptr;
		other->OnInputDefocus(); // Must call without focus
	}
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
		string msg = GetText();
		if (bIsSensitiveText)
			for (char& c : msg)
				c = '*';

		// TODO - Cache clamped text
		DrawText(window, GetClampedText(msg, !IsFocused()), GetTextColour(), IsFocused() ? GetTextStyle() : sf::Text::Italic);
	}
}

string UInputField::GetClampedText(const string& msg, const bool& clampFront, const bool& indicateTrail) const
{
	if (GetFont() == nullptr || msg.empty())
		return msg;

	// TEMP FIX : Treat every character as having a width of 12
	const uint32 maxChars = std::round(GetSize().x / 12.0f) + (indicateTrail ? -2 : 0);

	if (msg.size() > maxChars)
	{
		if (clampFront)
		{
			if (indicateTrail)
				return msg.substr(0, maxChars) + "..";
			else
				return msg.substr(0, maxChars);
		}
		else 
		{
			if (indicateTrail)
				return ".." + msg.substr(msg.size() - maxChars);
			else
				return msg.substr(msg.size() - maxChars);
		}
	}
	return msg;

	// Can use, but need to fix the implementation
	/*
	sf::Text text;
	text.setFont(*GetFont());
	text.setCharacterSize(GetFontSize());
	// Leaving scale as default, will scale with window
	if (GetScalingMode() == ScalingMode::PixelPerfect)
		text.setScale(vec2(1, 1));



	// Make sure text doesn't overflow
	text.setString((indicateTrail ? ".." : ""));
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
				text.setString((indicateTrail ? ".." : "") + msg[i] + clamped);
				if (text.getLocalBounds().width >= maxWidth)
					break;
				else
					clamped = msg[i] + clamped;
			}

			return (indicateTrail ? ".." : "") + clamped;
		}

		// Clamp from front
		else
		{
			string clamped = "";
			text.setString("");
			for (const char& c : msg)
			{
				text.setString(clamped + c + (indicateTrail ? ".." : ""));
				if (text.getLocalBounds().width >= maxWidth)
					break;
				else
					clamped += c;
			}
			return clamped + (indicateTrail ? ".." : "");
		}

	}
	return msg;
	*/
}