#include "Includes\Core\Camera.h"
CLASS_SOURCE(ACamera, CORE_API)


ACamera::ACamera()
{
}

#ifdef BUILD_CLIENT
void ACamera::OnDraw(sf::RenderWindow* window, const float& deltaTime)
{
	sf::View view = window->getView();
	view.setCenter(GetLocation());
	view.setSize(window->getSize().x, window->getSize().y);
	window->setView(view);

}
#endif