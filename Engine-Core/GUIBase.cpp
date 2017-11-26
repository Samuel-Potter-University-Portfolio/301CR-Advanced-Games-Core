#include "Includes\Core\GUIBase.h"
#include "Includes\Core\HUD.h"


CLASS_SOURCE(UGUIBase, CORE_API)
const vec2 UGUIBase::s_canvasSize(800, 800);


UGUIBase::UGUIBase() :
	m_location(0,0), m_size(100,100), m_anchor(0,0), m_origin(0,0),
	m_drawingLayer(0), m_colour(Colour::White), m_texture(nullptr)
{
}

void UGUIBase::OnLoaded(AHUD* hud) 
{
	m_parent = hud;
	OnBegin();
}

void UGUIBase::Draw(sf::RenderWindow* window, const float& deltaTime) 
{
	vec2 drawSize = GetDrawSize(window);
	sf::View view = sf::View(vec2(-m_anchor.x * drawSize.x * 0.5f, -m_anchor.y * drawSize.y * 0.5f), drawSize);
	window->setView(view);
	OnDraw(window, deltaTime);
}

vec2 UGUIBase::GetDrawSize(const sf::RenderWindow* window) const 
{
	const vec2 windowSize(window->getSize().x, window->getSize().y);
	const float aspect = windowSize.x / windowSize.y;

	vec2 drawSize(s_canvasSize.x * aspect, s_canvasSize.y);

	if (m_scalingMode == ScalingMode::PixelPerfect)
	{
		const float scale = windowSize.y / s_canvasSize.y;
		drawSize *= scale;
	}
	return drawSize;
}

void UGUIBase::OnDraw(sf::RenderWindow* window, const float& deltaTime) 
{
	DrawDefaultRect(window);
}

void UGUIBase::DrawDefaultRect(sf::RenderWindow* window)
{
	sf::RectangleShape rect;
	rect.setOrigin(m_origin);
	rect.setPosition(m_location);
	rect.setSize(m_size);
	rect.setFillColor(m_colour);
	rect.setTexture(m_texture);
	window->draw(rect);
}


void UGUIBase::HandleMouseOver(const MouseContainer& mouse)
{
	if (!bMouseWasOver)
	{
		bMouseWasOver = true;
		OnMouseEnter();
	}

	OnMouseOver();

	if (mouse.leftButton.IsPressed())
		OnMousePressed();
	if (mouse.leftButton.IsReleased())
		OnMouseReleased();
}

void UGUIBase::HandleMouseMiss(const struct MouseContainer& mouse) 
{
	if (bMouseWasOver)
	{
		bMouseWasOver = false;
		OnMouseExit();
	}
}


bool UGUIBase::IntersectRay(const ivec2& ray, const sf::RenderWindow* window) const
{
	if (!bIsVisible)
		return false;

	const vec2 drawSize = GetDrawSize(window);
	const vec2 windowSize(window->getSize().x, window->getSize().y);

	// Transform box into NDC
	sf::Transform view = sf::View(vec2(0,0), drawSize).getTransform();
	vec2 location;
	location.x = m_location.x - m_origin.x;
	location.y = m_origin.y - m_location.y;

	// Transfrom into corrected pixel space
	sf::FloatRect rect = view.transformRect(sf::FloatRect(location, m_size));
	rect.left += (0.5f + (rect.left + m_anchor.x) * 0.5f) * windowSize.x;
	rect.top += (0.5f + (rect.top + m_anchor.y + rect.height) * 0.5f) * windowSize.y;
	rect.width *= windowSize.x * 0.5f;
	rect.height *= windowSize.y * 0.5f;
	return rect.contains(vec2(ray.x, ray.y));
}