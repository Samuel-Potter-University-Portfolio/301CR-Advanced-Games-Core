#include "Includes\Core\Camera.h"


Camera::Camera()
{
	SetName("Camera");
}

#ifdef BUILD_CLIENT
void Camera::Draw(sf::RenderWindow* window, const float& deltaTime) 
{
	sf::View view = window->getView();
	view.setCenter(GetLocation());
	window->setView(view);
}
#endif