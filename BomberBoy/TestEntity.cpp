#include "TestEntity.h"



TestEntity::TestEntity()
{
}

#ifdef BUILD_CLIENT
void TestEntity::Draw(sf::RenderWindow* window, const float& deltaTime) 
{
	sf::CircleShape shape(50);
	shape.setFillColor(sf::Color(100, 255, 50));
	window->draw(shape);
}
#endif