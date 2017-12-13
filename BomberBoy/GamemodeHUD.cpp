#include "GamemodeHUD.h"

#include "ChatWidget.h"


CLASS_SOURCE(AGamemodeHUD)


void AGamemodeHUD::OnBegin()
{
	Super::OnBegin();
	bIsTickable = true;

	const sf::Font* defaultFont = GetAssetController()->GetFont("Resources\\UI\\coolvetica.ttf");
	const ULabel::ScalingMode& defaultScaling = ULabel::ScalingMode::Expand;


	UChatWidget* chat = AddElement<UChatWidget>();
	chat->SetFont(defaultFont);
}