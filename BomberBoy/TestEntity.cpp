#include "TestEntity.h"



TestEntity::TestEntity()
{
	bNetSynced = true;
}

bool TestEntity::FetchRPCIndex(const char* funcName, uint16& outID) const
{
	RPC_INDEX_HEADER(funcName, outID);
	RPC_INDEX(ResetPosition);
	RPC_INDEX(MoveTo);
	return false;
}

bool TestEntity::ExecuteRPC(uint16& id, ByteBuffer& params)
{
	RPC_EXEC_HEADER(id, params);
	RPC_EXEC(ResetPosition);
	RPC_EXEC_TwoParam(MoveTo, float, float);
	return false;
}

void TestEntity::OnBegin() 
{
	startPos = GetLocation();
}

void TestEntity::OnTick(const float& deltaTime) 
{
#ifdef BUILD_SERVER
	timer += deltaTime;

	if (timer >= 10.0f)
	{
		timer = 0.0f;
		CallRPC(TCP, RPCTarget::GlobalBroadcast, this, ResetPosition);
		return;
	}

	sf::Vector2f dl = GetLocation() + sf::Vector2f(30, 0) * deltaTime;
	CallRPC_TwoParam(TCP, RPCTarget::GlobalBroadcast, this, MoveTo, dl.x, dl.y);
#endif
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

void TestEntity::ResetPosition() 
{
	SetLocation(startPos);
}

void TestEntity::MoveTo(float x, float y) 
{
	SetLocation(sf::Vector2f(x, y));
}