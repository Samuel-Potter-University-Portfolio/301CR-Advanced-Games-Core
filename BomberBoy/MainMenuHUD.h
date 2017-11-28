#pragma once
#include "Core\Core-Common.h"

#include "LoginMenu.h"
#include "MenuContainer.h"
#include "HostMenu.h"
#include "ConnectMenu.h"



class AMainMenuHUD : public AHUD
{
	CLASS_BODY()
private:
	LoginMenu		m_loginMenu;
	HostMenu		m_hostMenu;
	ConnectMenu		m_connectMenu;
	MenuContainer	m_serverMenu;
	MenuContainer	m_settingsMenu;

	UButton* m_loginButton;
	UButton* m_connectButton;
	UButton* m_hostButton;
	UButton* m_serverButton;
	UButton* m_settingsButton;
	UButton* m_exitButton;

public:
	virtual void OnBegin() override;
	virtual void OnTick(const float& deltaTime) override;
};

