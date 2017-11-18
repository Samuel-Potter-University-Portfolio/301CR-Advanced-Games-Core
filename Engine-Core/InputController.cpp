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
					const bool currentState = m_keyStates[binding->key];
					binding->bIsPressed = !lastState && currentState;
					binding->bIsReleased = lastState && !currentState;
					binding->bIsHeld = currentState;
				}
	}
}

void InputController::UpdateKeystate(const sf::Event::KeyEvent& event, const bool& pressed)
{
	m_keyStates[event.code] = pressed;
}