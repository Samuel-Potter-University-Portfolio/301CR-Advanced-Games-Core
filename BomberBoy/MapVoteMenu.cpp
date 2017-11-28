#include "MapVoteMenu.h"


void MapVoteMenu::Build(AHUD* hud, const sf::Font* font, const ULabel::ScalingMode& scalingMode, const vec2 anchor) 
{
	SetupDefaultMenu(hud, "Map Voting", font, scalingMode, vec2(400, 630), anchor, vec2(1, 0));
	Game* game = hud->GetGame();
	m_levelController = (ALobbyController*)game->GetCurrentLevel()->GetLevelController();
	m_playerController = game->GetFirstObject<OBPlayerController>(true);


	// Cannot build without these two
	if (m_levelController == nullptr || m_playerController == nullptr)
	{
		LOG_ERROR("Missing requirements to build MapVoteMenu");
		return;
	}


	m_timerText = AddElement <ULabel>(hud);
	m_timerText->SetScalingMode(scalingMode);
	m_timerText->SetFont(font);
	m_timerText->SetText("Waiting on players..");
	m_timerText->SetFontSize(30);
	m_timerText->SetVerticalAlignment(ULabel::VerticalAlignment::Top);
	m_timerText->SetHorizontalAlignment(ULabel::HorizontalAlignment::Left);

	m_timerText->SetOrigin(vec2(0, 0));
	m_timerText->SetAnchor(anchor);
	m_timerText->SetLocation(vec2(-390, -60));
	m_timerText->SetSize(vec2(150, 150));

	
	// Map voting buttons
	for (uint32 i = 0; i < 6; ++i)
	{
		const bool lockedTile = i >= ALobbyController::s_supportedLevels.size();
		const int32 x = i % 2;
		const int32 y = i / 2;

		// Map voting
		UButton* button = AddElement<UButton>(hud);
		button->SetScalingMode(scalingMode);
		button->SetFont(font);
		button->SetText(lockedTile ? "" : ALobbyController::s_supportedLevels[i]->GetName());
		button->SetVerticalAlignment(ULabel::VerticalAlignment::Bottom);
		button->SetHorizontalAlignment(ULabel::HorizontalAlignment::Right);
		button->SetDisabled(lockedTile);
		button->SetDefaultColour(Colour::White);

		button->SetOrigin(vec2(0, 0));
		button->SetAnchor(anchor);
		button->SetLocation(vec2(-390 + 160 * x, 160 * y));
		button->SetSize(vec2(150, 150));

		const uint32 index = i;
		button->SetCallback([this, index]() { m_playerController->SetMapVote(index); });


		// Vote counter
		ULabel* label = AddElement<ULabel>(hud);
		label->SetScalingMode(scalingMode);
		label->SetFont(font);
		label->SetText("0 Votes");
		label->SetVerticalAlignment(ULabel::VerticalAlignment::Top);
		label->SetHorizontalAlignment(ULabel::HorizontalAlignment::Left);
		label->SetBlocksRaycasts(false);
		label->SetActive(!lockedTile);

		label->SetOrigin(vec2(0, 0));
		label->SetAnchor(anchor);
		label->SetLocation(vec2(-390 + 160 * x, 160 * y));
		label->SetSize(vec2(150, 150));
		m_levelVoteText.emplace_back(label);
	}


	// Toggle ready button
	UButton* readyButton = AddElement<UButton>(hud);
	readyButton->SetScalingMode(scalingMode);
	readyButton->SetFont(font);
	readyButton->SetText("Not Ready");
	readyButton->SetDefaultColour(Colour::Red);
	readyButton->SetVerticalAlignment(ULabel::VerticalAlignment::Top);
	readyButton->SetHorizontalAlignment(ULabel::HorizontalAlignment::Centre);


	readyButton->SetOrigin(vec2(0, 0));
	readyButton->SetAnchor(anchor);
	readyButton->SetLocation(vec2(-390, 160 * 3));
	readyButton->SetSize(vec2(150, 40));
	readyButton->SetCallback([this, readyButton]()
	{
		if (m_playerController->IsReady())
		{
			readyButton->SetText("Not Ready");
			readyButton->SetDefaultColour(Colour::Red);
			m_playerController->SetReady(false);
		}
		else
		{
			readyButton->SetText("Ready");
			readyButton->SetDefaultColour(Colour::Green);
			m_playerController->SetReady(true);
		}
	});

}

void MapVoteMenu::UpdateDisplay(AHUD* hud) 
{
	// Invalid without either of these two
	if (m_levelController == nullptr || m_playerController == nullptr)
		return;

	// Update timer text
	if(!m_levelController->IsTimerActive())
		m_timerText->SetText("Waiting on players..");
	else
	{
		const uint32 time = (uint32)m_levelController->GetTimeUntilStart();
		uint32 minutes = time / 60;
		uint32 seconds = time % 60;

		const string timeStamp = std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);
		m_timerText->SetText(timeStamp + " remaining to vote..");

		if (time < 10)
			m_timerText->SetTextColour(Colour::Red);
		else
			m_timerText->SetTextColour(Colour::Black);
	}

	// Update vote counts
	for (uint32 i = 0; i < m_levelVoteText.size(); ++i)
		m_levelVoteText[i]->SetText(std::to_string(m_levelController->GetMapVotes(i)) + " Votes");
}