#include "ChatWidget.h"


CLASS_SOURCE(UChatWidget)
UChatWidget* UChatWidget::s_main = nullptr;



UChatWidget::UChatWidget()
{
	SetAnchor(vec2(-1, 1));
	SetSize(vec2(350, 30));
	SetOrigin(vec2(0, GetSize().y));
	SetLocation(vec2(10, -10));
	SetFontSize(20);

	SetDefaultColour(Colour(0, 0, 0, 255));
	SetTextColour(Colour(255, 255, 255, 255));
	SetDefaultText(" Click here chat");
}

UChatWidget::~UChatWidget() 
{
	if (s_main == this)
		s_main = nullptr;
}

void UChatWidget::OnBegin() 
{
	Super::OnBegin();
	if (s_main != nullptr)
		LOG_WARNING("Multiple UChatWidgets detected active at once (Using most recent)");
	s_main = this;


	m_localPlayer = GetHUD()->GetGame()->GetFirstObject<OBPlayerController>(true);
	if (m_localPlayer == nullptr)
		LOG_ERROR("No local player found! UChatWidget requires one!");
}

void UChatWidget::OnType() 
{
	Super::OnType();
	string text = GetText();
	if (text.size() >= STR_MAX_ENCODE_LEN)
		SetText(text.substr(0, STR_MAX_ENCODE_LEN - 1));
}

void UChatWidget::OnInputDefocus()
{
	Super::OnInputDefocus();

	if (IsFocused())
	{
		string message = GetText();
		CallRPC_OneParam(m_localPlayer, SendMessage, message);
	}

	SetText("");
}

void UChatWidget::OnDraw(sf::RenderWindow* window, const float& deltaTime) 
{
	Super::OnDraw(window, deltaTime);

		
	for (int32 i = 0; i < CHAT_LOG_SIZE; ++i)
	{
		const uint32 index = (CHAT_LOG_SIZE - i + m_chatHead) % CHAT_LOG_SIZE;
		DrawText(window, m_chatLog[index], GetTextColour(), GetTextStyle(), vec2(0, -(1 + i) * GetSize().y));
	}
}

void UChatWidget::LogMessage(OBPlayerController* player, const string& message)
{
	string playerName;
	if (player != nullptr)
		playerName = "[" + player->GetDisplayName() + "]";


	// Format message, to put on multiple lines, if needed
	string msg = playerName + ": " + message;
	string line;
	
	while (!msg.empty()) 
	{
		line = GetClampedText(msg, true, false);
		m_chatHead = (m_chatHead + 1) % CHAT_LOG_SIZE;
		m_chatLog[m_chatHead] = line;
		msg = msg.substr(line.size());
	}
}