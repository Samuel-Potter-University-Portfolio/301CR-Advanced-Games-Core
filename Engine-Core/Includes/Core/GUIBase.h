#pragma once
#include "Common.h"
#include "ManagedClass.h"
#include <SFML/Graphics.hpp>


/**
* GUIs are graphical elements that will only be spawned on clients
* All children of this class should append an U to the beginning of their name
*/
class CORE_API UGUIBase : public ManagedObject
{
	CLASS_BODY()
public:
	/// The canvas scale that everything will be based/scale on
	static const vec2 s_canvasSize;

	enum ScalingMode 
	{
		PixelPerfect,
		Expand
	};

protected:
	uint8 m_drawingLayer;

private:
	ScalingMode m_scalingMode = ScalingMode::PixelPerfect;
	vec2 m_location;
	vec2 m_origin;
	vec2 m_size;
	vec2 m_anchor;

public:
	UGUIBase();
	virtual ~UGUIBase() {}


	/**
	* Called when this GUI should be drawn to the screen
	* @param window			The window to draw to
	* @param deltaTime		Time since last draw in seconds)
	*/
	void Draw(sf::RenderWindow* window, const float& deltaTime);

protected:
	/**
	* Callback for when this object is drawn (Allows for custom drawing behaviour)
	* -Note: Default implementation will draw rect with applied texture
	* @param window			The window to draw to
	* @param deltaTime		Time since last draw in seconds)
	*/
	virtual void OnDraw(sf::RenderWindow* window, const float& deltaTime);


	/**
	* Getters & Setters
	*/
public:
	inline const uint8& GetDrawingLayer() const { return m_drawingLayer; }

	inline void SetScalingMode(const ScalingMode& value) { m_scalingMode = value; }
	inline const ScalingMode& GetScalingMode() const { return m_scalingMode; }


	inline void SetLocation(const vec2& value) { m_location = value; }
	inline const vec2& GetLocation() const { return m_location; }

	inline void SetOrigin(const vec2& value) { m_origin = value; }
	inline const vec2& GetOrigin() const { return m_origin; }

	inline void SetSize(const vec2& value) { m_size = value; }
	inline const vec2& GetSize() const { return m_size; }

	inline void SetAnchor(const vec2& value) { m_anchor = value; }
	inline const vec2& GetAnchor() const { return m_anchor; }

};