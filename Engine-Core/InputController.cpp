#include "Includes\Core\InputController.h"
#include "Includes\Core\Game.h"


InputController::InputController()
{
}

InputController::~InputController()
{
}

void InputController::HandleUpdate(Game* game, const float& deltaTime)
{
	LLevel* level = game->GetCurrentLevel();
	if (level != nullptr)
	{
		std::vector<AActor*> actors = level->GetActiveActors();

		// Update all actors
		for (AActor* actor : actors)

			// Only update actor, if locally controller and have bindings registered
			if (actor->IsNetOwner() && actor->CanReceiveInput() && !actor->IsDestroyed())
				for (KeyBinding* binding : actor->GetKeyBindings()) 
				{
					const bool lastState = binding->bIsHeld;
					bool currentState;

					if (binding->m_bindingMode == KeyBinding::BindingMode::Keyboard)
						currentState = m_keyStates[binding->m_key];
					else
						currentState = m_mouseStates[binding->m_button];

					binding->bIsPressed = !lastState && currentState;
					binding->bIsReleased = lastState && !currentState;
					binding->bIsHeld = currentState;
				}
	}
}

void InputController::UpdateEvent(const sf::Event& event)
{
	switch (event.type)
	{
		case sf::Event::KeyPressed:
			m_keyStates[event.key.code] = true;
			break;
		case sf::Event::KeyReleased:
			m_keyStates[event.key.code] = false;
			break;

		case sf::Event::MouseButtonPressed:
			m_mouseStates[event.mouseButton.button] = true;
			break;
		case sf::Event::MouseButtonReleased:
			m_mouseStates[event.mouseButton.button] = false;
			break;

		case sf::Event::MouseWheelMoved:
			break;

		case sf::Event::MouseMoved:
			m_mousePosition.x = event.mouseMove.x;
			m_mousePosition.y = event.mouseMove.y;
			break;

		case sf::Event::JoystickButtonPressed:
			break;
		case sf::Event::JoystickButtonReleased:
			break;
		case sf::Event::JoystickMoved:
			break;
	}
}