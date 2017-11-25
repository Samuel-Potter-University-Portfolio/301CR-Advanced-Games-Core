#include "Includes\Core\HUD.h"
#include "Includes\Core\Engine.h"
#include "Includes\Core\Game.h"


CLASS_SOURCE(AHUD, CORE_API)


AHUD::AHUD()
{
	bIsNetSynced = false;
	RegisterKeybinding(&m_mouse.leftButton);
	RegisterKeybinding(&m_mouse.rightButton);
	RegisterKeybinding(&m_mouse.middleButton);
}

AHUD::~AHUD()
{
	for (UGUIBase* gui : m_elements)
		delete gui;
}

void AHUD::OnTick(const float& deltaTime) 
{
}

void AHUD::DisplayUpdate(sf::RenderWindow* window, const float& deltaTime)
{
	// Attempt to handle mouse events
	{
		m_mouse.location = GetInputController()->GetMouseLocation();

		// Update the mouse event state of all elements
		bool hit = false;
		for (uint32 layer = 0; layer < 10; ++layer)
		{
			for (uint32 i = 0; i < m_elements.size(); ++i)
			{
				// Go through layers from top to bottom
				UGUIBase* elem = m_elements[i];
				if (elem->GetDrawingLayer() == 9 - layer)
				{
					// Attempt to cast ray at topmost element
					if (!hit && elem->BlocksRaycasts() && elem->IntersectRay(m_mouse.location, window))
					{
						elem->HandleMouseOver(m_mouse);
						hit = true;
					}
					else
						elem->HandleMouseMiss(m_mouse);
				}
			}
		}
	}

	// Update all elements
	for (UGUIBase* elem : m_elements)
	{
		if (elem->IsTickable())
			elem->OnTick(deltaTime);
	}


	// TODO - Just store elements in layers to start with
	for (uint32 layer = 0; layer < 10; ++layer)
		for (uint32 i = 0; i < m_elements.size(); ++i)
		{
			UGUIBase* elem = m_elements[i];
			if (elem->IsVisible() && elem->GetDrawingLayer() == layer)
				elem->Draw(window, deltaTime);
		}
}

UGUIBase* AHUD::AddElement(SubClassOf<UGUIBase> type)
{ 
	UGUIBase* gui = type->New<UGUIBase>();
	m_elements.push_back(gui); 
	gui->OnLoaded(this);
	return gui;
}

const InputController* AHUD::GetInputController() const 
{ 
	return GetGame()->GetEngine()->GetInputController(); 
}