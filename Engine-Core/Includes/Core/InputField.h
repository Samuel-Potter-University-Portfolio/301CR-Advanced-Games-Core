#pragma once
#include "Label.h"
#include <functional>


/**
* Callback for when a field either loses focus or has it's data entered
*/
typedef std::function<void(string)> FieldCallback;


/**
* Represents a text input field that the user may focus on and type in
*/
class CORE_API UInputField : public ULabel
{
	CLASS_BODY()
private:
	static UInputField* s_currentFocus;
	FieldCallback m_callback;

	Colour m_defaultColour;
	Colour m_disabledColour;

	bool bIsSensitiveText = false;
	string m_defaultText = "Input";

public:
	UInputField();


	virtual void OnTick(const float& deltaTime) override;
	virtual void OnMousePressed() override;

	/**
	* Callback for when the user focuses on this field
	*/
	virtual void OnInputFocus() {}
	/**
	* Callback for when the user defocusses on this field
	* May be called for user pressing return or clicking off of the field
	*/
	virtual void OnInputDefocus();

	/**
	* Callback for when the user types anything in this field
	*/
	virtual void OnType() {}

	/**
	* Called when this GUI should be drawn to the screen
	* @param window			The window to draw to
	* @param deltaTime		Time since last draw in seconds
	*/
	void OnDraw(sf::RenderWindow* window, const float& deltaTime) override;

	/**
	* Get the clamped texts to display
	* @param msg				The message to clamp
	* @param clampFront			To clamp from the front or back
	* @param indicateTrail		Put elipsis at the trail
	* @returns Text that is safe to display
	*/
	string GetClampedText(const string& msg, const bool& clampFront, const bool& indicateTrail = true) const;


	/**
	* Getters & Setters
	*/
public:
	inline void SetCallback(FieldCallback callback) { m_callback = callback; }
	inline bool IsFocused() const { return s_currentFocus == this && !IsDisabled(); }

	inline void SetSensitiveText(const bool& value) { bIsSensitiveText = value; }
	inline bool IsSensitiveText() const { return bIsSensitiveText; }

	inline void SetDefaultColour(const Colour& value) { m_defaultColour = value; }
	inline const Colour& GetDefaultColour() const { return m_defaultColour; }

	inline void SetDisabledColour(const Colour& value) { m_disabledColour = value; }
	inline const Colour& GetDisabledColour() const { return m_disabledColour; }

	inline void SetDefaultText(const string& value) { m_defaultText = value; }
	inline const string& GetDefaultText() const { return m_defaultText; }
};

