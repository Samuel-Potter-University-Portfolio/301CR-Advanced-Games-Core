#include "LobbyHUD.h"

#include "ChatWidget.h"


CLASS_SOURCE(ALobbyHUD)


ALobbyHUD::ALobbyHUD()
{
	bIsTickable = true;
}

void ALobbyHUD::OnBegin() 
{
	Super::OnBegin();
	const sf::Font* defaultFont = GetAssetController()->GetFont("Resources\\UI\\coolvetica.ttf");
	const ULabel::ScalingMode& defaultScaling = ULabel::ScalingMode::Expand;

	// Title
	{
		ULabel* title = AddElement<ULabel>();
		title->SetFont(defaultFont);
		title->SetScalingMode(defaultScaling);
		title->SetText("Lobby");
		title->SetTextColour(Colour::Blue);
		title->SetFontSize(100);
		title->SetHorizontalAlignment(ULabel::HorizontalAlignment::Centre);
		title->SetVerticalAlignment(ULabel::VerticalAlignment::Top);

		title->SetLocation(vec2(0, 30));
		title->SetOrigin(vec2(5, 5));
		title->SetSize(vec2(10, 10));
		title->SetAnchor(vec2(0, -1));


		ULabel* icon = AddElement<ULabel>();
		icon->SetScalingMode(defaultScaling);
		icon->SetTexture(GetAssetController()->GetTexture("Resources\\Items\\Default_Bomb.png.1"));
		icon->SetDrawBackground(true);

		icon->SetLocation(vec2(-40, 110));
		icon->SetSize(vec2(80, 80));
		icon->SetOrigin(vec2(50, 50));
		icon->SetAnchor(vec2(0, -1));
	}


	// Setup all player cards
	for (uint32 i = 0; i < 16; ++i)
	{
		m_playerCards[i].Build(this, defaultFont, defaultScaling, i);
		m_playerCards[i].SetPlayer(nullptr);
	}


	// Lock all ones that cannot be used
	NetSession* session = GetGame()->GetSession();
	if (session != nullptr)
	{
		for (uint32 i = 0; i < 16 - session->GetMaxPlayerCount(); ++i)
			m_playerCards[15 - i].SetLockedStyle();
	}


	UChatWidget* chat = AddElement<UChatWidget>();
	chat->SetFont(defaultFont);
}

void ALobbyHUD::OnTick(const float& deltaTime) 
{
	Super::OnTick(deltaTime);

	// Update all cards
	for (PlayerCard& card : m_playerCards)
		card.UpdateDisplay(this);
}

void ALobbyHUD::OnPlayerConnect(OBPlayerController* player) 
{
	if (m_localPlayer == nullptr && player->IsNetOwner())
		m_localPlayer = player;

	// Give the player a tag
	for (uint32 i = 0; i < 16; ++i)
	{
		if (m_playerCards[i].m_player == nullptr)
		{
			m_playerCards[i].SetPlayer(player);
			return;
		}
	}
}

void ALobbyHUD::OnPlayerDisconnect(OBPlayerController* player) 
{
	if (m_localPlayer == player)
		m_localPlayer = nullptr;

	// Remove the player tag
	for (uint32 i = 0; i < 16; ++i)
	{
		if (m_playerCards[i].m_player == player)
		{
			m_playerCards[i].SetPlayer(nullptr);
			return;
		}
	}
}


void PlayerCard::Build(AHUD* hud, const sf::Font* font, const ULabel::ScalingMode& scalingMode, const uint32& index)
{
	const vec2 anchor(-1, -0.55f);
	const vec2 location(10 + 310 * ((index) % 2), 60 * ((index) / 2));

	m_background = AddElement<ULabel>(hud);
	m_background->SetScalingMode(scalingMode);
	m_background->SetDrawBackground(true);
	m_background->SetColour(Colour(200, 200, 200, 255));

	m_background->SetSize(vec2(300, 50));
	m_background->SetOrigin(vec2(0, 0));
	m_background->SetAnchor(anchor);
	m_background->SetLocation(location);


	m_icon = AddElement<ULabel>(hud);
	m_icon->SetScalingMode(scalingMode);
	m_icon->SetDrawBackground(true);
	m_icon->SetColour(Colour(255, 255, 255, 255));
	m_animation = hud->GetAssetController()->GetAnimation("Resources\\Character\\Down.anim");
	if (m_animation != nullptr)
		m_icon->SetTexture(m_animation->GetFrame(0));

	m_icon->SetSize(vec2(32, 41) * 1.5f);
	m_icon->SetOrigin(vec2(0, 0));
	m_icon->SetAnchor(anchor);
	m_icon->SetLocation(location + vec2(5, -8));


	m_name = AddElement<ULabel>(hud);
	m_name->SetScalingMode(scalingMode);
	m_name->SetFont(font);
	m_name->SetFontSize(40);
	m_name->SetText("Player Name");
	m_name->SetVerticalAlignment(ULabel::VerticalAlignment::Top);
	m_name->SetHorizontalAlignment(ULabel::HorizontalAlignment::Left);

	m_name->SetSize(vec2(50, 50));
	m_name->SetOrigin(vec2(0, 0));
	m_name->SetAnchor(anchor);
	m_name->SetLocation(location + vec2(50, -5));
}

void PlayerCard::SetPlayer(OBPlayerController* player)
{
	if (player == nullptr)
	{
		m_background->SetColour(Colour(50, 50, 50, 255));
		m_icon->SetColour(Colour(0, 0, 0, 255));
		m_name->SetText("");
	}
	else
	{
		if (player->IsNetOwner())
			m_background->SetColour(Colour(250, 250, 250, 255));
		else
			m_background->SetColour(Colour(200, 200, 200, 255));

		m_icon->SetColour(Colour(255, 255, 255, 255));
		m_name->SetText(player->GetDisplayName());
	}
	m_player = player;
}

void PlayerCard::UpdateDisplay(AHUD* hud)
{
	if (m_player == nullptr)
		return;

	// Update colour of icon
	if (m_colourIndex != m_player->GetColourIndex())
	{
		m_colourIndex = m_player->GetColourIndex();
		string code = m_player->GetColourCode();
		m_animation = hud->GetAssetController()->GetAnimation("Resources\\Character\\Down.anim." + code);
	}

	// TODO - Only animate if ready
	m_icon->SetTexture(m_animation->GetCurrentFrame());
}

void PlayerCard::SetLockedStyle()
{
	m_background->SetColour(Colour(30, 30, 30, 255));
	m_icon->SetActive(false);
	m_name->SetText("");
}