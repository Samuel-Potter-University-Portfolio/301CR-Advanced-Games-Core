#pragma once
#include "Core\Core-Common.h"

#include "MenuContainer.h"
#include "HostMenu.h"



class AMainMenuHUD : public AHUD
{
	CLASS_BODY()
private:
	MenuContainer	m_loginMenu;
	HostMenu		m_hostMenu;
	MenuContainer	m_connectMenu;
	MenuContainer	m_serverMenu;
	MenuContainer	m_settingsMenu;

public:
	virtual void OnBegin() override;
};

