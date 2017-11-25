#include "Includes\Core\GUIBase.h"


CLASS_SOURCE(UGUIBase, CORE_API)
const vec2 UGUIBase::s_canvasSize(1000, 1000);


UGUIBase::UGUIBase() :
	m_location(0,0), m_size(100,100), m_anchor(0,0), m_origin(50,50),
	m_drawingLayer(0)
{
}

void UGUIBase::Draw(sf::RenderWindow* window, const float& deltaTime) 
{
	// Draw from correct position
	const vec2 windowSize(window->getSize().x, window->getSize().y);
	const float aspect = windowSize.x / windowSize.y;

	vec2 drawSize(s_canvasSize.x * aspect, s_canvasSize.y);
		
	if (m_scalingMode == ScalingMode::PixelPerfect)
	{
		const float scale = windowSize.y / s_canvasSize.y;
		drawSize *= scale;
	}


	sf::View view = sf::View(vec2(-m_anchor.x * drawSize.x * 0.5f, -m_anchor.y * drawSize.y * 0.5f), drawSize);
	window->setView(view);
	OnDraw(window, deltaTime);
}

void UGUIBase::OnDraw(sf::RenderWindow* window, const float& deltaTime) 
{
	sf::RectangleShape rect;
	rect.setOrigin(m_origin);
	rect.setPosition(m_location);
	rect.setSize(m_size);

	// TODO - Add texture and such
	window->draw(rect);
}