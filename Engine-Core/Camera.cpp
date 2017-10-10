#include "Includes\Core\Camera.h"


Camera::Camera()
{
	SetName("Camera");
}

#ifdef BUILD_CLIENT
void Camera::Draw(sf::RenderWindow* window, const float& deltaTime) 
{
	//window->setView(sf::View());

	//sf::CircleShape shape(50);
	//shape.setPosition(sf::Vector2f(100, 100));
	//shape.setFillColor(sf::Color(100, 255, 50));
	//window->draw(shape);
}
#endif