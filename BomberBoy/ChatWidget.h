#pragma once
#include "Core\Core-Common.h"
#include "BPlayerController.h"


#define CHAT_LOG_SIZE 10


class UChatWidget : public UInputField
{
	CLASS_BODY()
public:
	static UChatWidget* s_main;

private:
	OBPlayerController* m_localPlayer = nullptr;

	// Looping log
	uint32 m_chatHead = 0;
	string m_chatLog[CHAT_LOG_SIZE];

public:
	UChatWidget();
	virtual ~UChatWidget();

	virtual void OnBegin() override;
	virtual void OnDraw(sf::RenderWindow* window, const float& deltaTime) override;


	/** Callback for when the user types anything here */
	virtual void OnType() override;
	/** Callback for when the user finishes typeing */
	virtual void OnInputDefocus() override;

	/**
	* Callback for when the player sends a message
	* @param player				The controller of the player, who sent it
	* @param message			The message that has been received
	*/
	void LogMessage(OBPlayerController* player, const string& message);
};

