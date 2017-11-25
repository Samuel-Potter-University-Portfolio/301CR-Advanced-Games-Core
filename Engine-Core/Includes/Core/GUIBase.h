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
	bool bIsTickable = false;
	bool bIsVisible = true;
	uint8 m_drawingLayer;

private:
	bool bMouseWasOver = false;

	ScalingMode m_scalingMode = ScalingMode::PixelPerfect;
	vec2 m_location;
	vec2 m_origin;
	vec2 m_size;
	vec2 m_anchor;

	Colour m_colour;
	sf::Texture* m_texture = nullptr;
	
public:
	UGUIBase();
	virtual ~UGUIBase() {}


	/**
	* Called after before this is drawn
	* @param deltaTime		Time since last draw in seconds)
	*/
	virtual void OnTick(const float& deltaTime) {}
	
	/**
	* Called when this GUI should be drawn to the screen
	* @param window			The window to draw to
	* @param deltaTime		Time since last draw in seconds)
	*/
	void Draw(sf::RenderWindow* window, const float& deltaTime);
	/**
	* Draw the default rectangle using applied settings
	* @param window			The window to draw to
	*/
	void DrawDefaultRect(sf::RenderWindow* window);
	/**
	* Get the draw size that will be used for this object
	* @param window			The window that will be drawn to
	*/
	vec2 GetDrawSize(const sf::RenderWindow* window) const;


	/**
	* Callback for when the mouse hovers over this
	*/
	void HandleMouseOver(const struct MouseContainer& mouse);
	/**
	* Callback for when the mouse misses this element
	*/
	void HandleMouseMiss(const struct MouseContainer& mouse);

	/**
	* Does this element intersect with this ray
	* @param ray			The ray in question
	* @param window			The window that is casting the ray
	*/
	virtual bool IntersectRay(const ivec2& ray, const sf::RenderWindow* window) const;

protected:
	/** Callback for when the mouse hovers over this element */
	virtual void OnMouseOver() {}

	/** Callback for when the mouse enters this element */
	virtual void OnMouseEnter() {}
	/** Callback for when the mouse exits this element */
	virtual void OnMouseExit() {}

	/** Callback for when the mouse clicks when over this element */
	virtual void OnMousePressed() {}
	/** Callback for when the mouse clicks when over this element */
	virtual void OnMouseReleased() {}


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
	inline uint8 GetDrawingLayer() const { return m_drawingLayer; }
	inline bool IsTickable() const { return bIsTickable; }

	inline bool IsVisible() const { return bIsVisible; }
	inline void SetVisible(const bool& value) { bIsVisible = value; }

	inline void SetScalingMode(const ScalingMode& value) { m_scalingMode = value; }
	inline ScalingMode GetScalingMode() const { return m_scalingMode; }


	inline void SetLocation(const vec2& value) { m_location = value; }
	inline const vec2& GetLocation() const { return m_location; }

	inline void SetOrigin(const vec2& value) { m_origin = value; }
	inline const vec2& GetOrigin() const { return m_origin; }

	inline void SetSize(const vec2& value) { m_size = value; }
	inline const vec2& GetSize() const { return m_size; }

	inline void SetAnchor(const vec2& value) { m_anchor = value; }
	inline const vec2& GetAnchor() const { return m_anchor; }

	inline void SetColour(const Colour& value) { m_colour = value; }
	inline const Colour& GetColour() const { return m_colour; }

	inline void SetTexture(sf::Texture* value) { m_texture = value; }
	inline sf::Texture* GetTexture() const { return m_texture; }
};