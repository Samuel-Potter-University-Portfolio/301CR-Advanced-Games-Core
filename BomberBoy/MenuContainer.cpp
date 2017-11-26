#include "MenuContainer.h"


void MenuContainer::SetupDefaultMenu(AHUD* parent, const string& title, const sf::Font* font, const ULabel::ScalingMode& scalingMode, const vec2& size, const vec2& anchor)
{
	ULabel* background = AddElement<ULabel>(parent);
	background->SetScalingMode(scalingMode);
	background->SetAnchor(anchor);
	background->SetLocation(vec2(0, -100));
	background->SetSize(size);
	background->SetColour(Colour(200, 200, 200, 255));
	background->SetOrigin(vec2(background->GetSize().x * 0.5f, 0.0f));
	background->SetDrawBackground(true);

	ULabel* titleLabel = AddElement<ULabel>(parent);
	titleLabel->SetScalingMode(scalingMode);
	titleLabel->SetFont(font);
	titleLabel->SetText(title);
	titleLabel->SetFontSize(40);

	titleLabel->SetAnchor(anchor);
	titleLabel->SetLocation(vec2(0, -100));
	titleLabel->SetSize(vec2(size.x, 40));
	titleLabel->SetColour(Colour(150, 150, 150, 255));
	titleLabel->SetOrigin(vec2(size.x * 0.5f, 0.0f));
	titleLabel->SetDrawBackground(true);
}