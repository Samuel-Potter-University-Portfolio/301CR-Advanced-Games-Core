#include "MenuContainer.h"


void MenuContainer::SetupDefaultMenu(AHUD* parent, const string& title, const sf::Font* font, const ULabel::ScalingMode& scalingMode, const vec2& size, const vec2& anchor, const vec2& localOrigin)
{
	const vec2 origin = vec2(localOrigin.x * size.x, localOrigin.y * size.y);

	ULabel* background = AddElement<ULabel>(parent);
	background->SetScalingMode(scalingMode);
	background->SetAnchor(anchor);
	background->SetLocation(vec2(0, -100));
	background->SetSize(size);
	background->SetColour(Colour(200, 200, 200, 255));
	background->SetOrigin(origin);
	background->SetDrawBackground(true);


	ULabel* titleBackground = AddElement<ULabel>(parent);
	titleBackground->SetScalingMode(scalingMode);
	titleBackground->SetOrigin(origin);

	titleBackground->SetAnchor(anchor);
	titleBackground->SetLocation(vec2(0, -100));
	titleBackground->SetSize(vec2(size.x, 40));
	titleBackground->SetColour(Colour(150, 150, 150, 255));
	titleBackground->SetOrigin(origin);
	titleBackground->SetDrawBackground(true);


	ULabel* titleLabel = AddElement<ULabel>(parent);
	titleLabel->SetScalingMode(scalingMode);
	titleLabel->SetFont(font);
	titleLabel->SetText(title);
	titleLabel->SetOrigin(origin);
	titleLabel->SetFontSize(37);
	titleLabel->SetVerticalAlignment(ULabel::VerticalAlignment::Top);

	titleLabel->SetAnchor(anchor);
	titleLabel->SetLocation(vec2(0, -110));
	titleLabel->SetSize(vec2(size.x, 40));
	titleLabel->SetOrigin(origin);
}