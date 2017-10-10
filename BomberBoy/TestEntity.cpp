#include "TestEntity.h"



TestEntity::TestEntity()
{
}

void TestEntity::OnBegin() 
{

}

void TestEntity::OnTick(const float& deltaTime) 
{
	SetLocation(GetLocation() + sf::Vector2f(10, 0) * deltaTime);
}

#ifdef BUILD_CLIENT
void TestEntity::Draw(sf::RenderWindow* window, const float& deltaTime) 
{
	sf::CircleShape shape(50);
	shape.setPosition(GetLocation());
	shape.setFillColor(sf::Color(100, 255, 50));
	window->draw(shape);
}
#endif