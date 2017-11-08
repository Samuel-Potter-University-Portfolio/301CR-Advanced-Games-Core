#include "TestEntity.h"



TestEntity::TestEntity()
{
	bNetSynced = true;
}

bool TestEntity::FetchRPCIndex(const char* funcName, uint16& outID) const
{
	RPC_INDEX_HEADER(funcName, outID);
	return false;
}

bool TestEntity::ExecuteRPC(uint16& id, ByteBuffer& params)
{
	RPC_EXEC_HEADER(id, params);
	return false;
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
