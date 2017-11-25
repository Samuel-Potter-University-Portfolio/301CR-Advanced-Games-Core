#pragma once
#include "Actor.h"
#include "GUIBase.h"
#include "InputController.h"


/**
* Simple container for holding mouse info
*/
struct MouseContainer 
{
	ivec2		location;
	KeyBinding	leftButton		= KeyBinding(sf::Mouse::Button::Left);
	KeyBinding	rightButton		= KeyBinding(sf::Mouse::Button::Right);
	KeyBinding	middleButton	= KeyBinding(sf::Mouse::Button::Middle);

	inline bool HasAnyInput() const { return leftButton.IsHeld() || rightButton.IsHeld() || middleButton.IsHeld(); }
};

/**
* Contains all GUI elements and only exists on clients
*/
class CORE_API AHUD : public AActor
{
	CLASS_BODY()
private:
	std::vector<UGUIBase*> m_elements;
	MouseContainer m_mouse;

public:
	AHUD();
	virtual ~AHUD();

	virtual void OnTick(const float& deltaTime) override;

	/**
	* Called after all actors have been drawn
	* @param window			The window to draw to
	* @param deltaTime		Time since last draw in seconds)
	*/
	void DisplayUpdate(sf::RenderWindow* window, const float& deltaTime);


	/**
	* Add a GUI element to the HUD
	* @param type			The GUI type to add
	* @returns The new object (Or nullptr if failed)
	*/
	UGUIBase* AddElement(SubClassOf<UGUIBase> type);

	/**
	* Add a GUI element to the HUD
	* @returns The new object (Or nullptr if failed)
	*/
	template<class Type>
	inline Type* AddElement() { return static_cast<Type*>(AddElement(Type::StaticClass())); }

	/**
	* Getters & Setters
	*/
public:
	const InputController* GetInputController() const;
	inline const MouseContainer* GetMouse() const { return &m_mouse; }
};

