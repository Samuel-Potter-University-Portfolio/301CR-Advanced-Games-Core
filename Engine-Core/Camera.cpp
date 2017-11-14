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
	window->setView(view);
}
#endif