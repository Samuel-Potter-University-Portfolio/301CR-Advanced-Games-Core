#include "Includes\Core\Button.h"


CLASS_SOURCE(UButton, CORE_API)


UButton::UButton() :
	m_defaultColour(200, 200, 200, 255),
	m_enteredColour(235, 235, 235, 255),
	m_pressedColour(255, 255, 255, 255), 
	m_disabledColour(50, 50, 50, 255)
{
	SetColour(m_defaultColour);
	SetText("Button");
	SetDrawBackground(true);

	SetHorizontalAlignment(ULabel::HorizontalAlignment::Centre);
	SetVerticalAlignment(ULabel::VerticalAlignment::Middle);
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
	bPressedOnThis = false;
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

	// Change style of text, if disabled
	if (IsDisabled())
	{
		Colour col = GetTextColour();
		col.a = 150;
		SetTextColour(col);
	}
	else
	{
		Colour col = GetTextColour();
		col.a = 255;
		SetTextColour(col);
	}

	Super::OnDraw(window, deltaTime);
}