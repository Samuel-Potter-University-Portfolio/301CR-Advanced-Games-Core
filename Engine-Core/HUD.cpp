#include "Includes\Core\HUD.h"
#include "Includes\Core\Engine.h"
#include "Includes\Core\Game.h"


CLASS_SOURCE(AHUD, CORE_API)


AHUD::AHUD()
{
	bIsNetSynced = false;
}

AHUD::~AHUD()
{
	for (UGUIBase* gui : m_elements)
		delete gui;
}

void AHUD::DisplayUpdate(sf::RenderWindow* window, const float& deltaTime)
{
	// TODO - Update GUI logic!
	// Mouse update: GetGame()->GetEngine()->GetInputerController()

	// TODO - Just store elements in layers to start with
	for (uint32 layer = 0; layer < 10; ++layer)
		for (uint32 i = 0; i < m_elements.size(); ++i)
		{
			if (m_elements[i]->GetDrawingLayer() == layer)
				m_elements[i]->Draw(window, deltaTime);
		}
}

UGUIBase* AHUD::AddElement(SubClassOf<UGUIBase> type)
{ 
	UGUIBase* gui = type->New<UGUIBase>();
	m_elements.push_back(gui); 
	return gui;
}