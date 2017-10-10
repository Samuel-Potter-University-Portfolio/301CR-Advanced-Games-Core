#include "Includes\Core\Camera.h"


Camera::Camera()
{
	SetName("Camera");
}

#ifdef BUILD_CLIENT
void Camera::Draw(sf::RenderWindow* window, const float& deltaTime) 
{
	window->setView(sf::View());
}
#endif