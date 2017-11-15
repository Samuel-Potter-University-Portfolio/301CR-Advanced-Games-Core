#include "TestEntity.h"
CLASS_SOURCE(ATestEntity)

#include "Core\Level.h"
#include "Core\Game.h"
#include "Core\PlayerController.h"

ATestEntity::ATestEntity()
{
	bIsNetSynced = true;
	bIsTickable = true;
}
bool ATestEntity::RegisterRPCs(const char* func, RPCInfo& outInfo) const
{
	RPC_INDEX_HEADER(func, outInfo);
	RPC_INDEX(TCP, RPCCallingMode::Broadcast, ResetPosition);
	RPC_INDEX(UDP, RPCCallingMode::Broadcast, MoveTo);
	RPC_INDEX(UDP, RPCCallingMode::Broadcast, PrintTime);
	return false;
}

bool ATestEntity::ExecuteRPC(uint16& id, ByteBuffer& params)
{
	RPC_EXEC_HEADER(id, params);
	RPC_EXEC(ResetPosition);
	RPC_EXEC_TwoParam(MoveTo, float, float);
	RPC_EXEC_TwoParam(PrintTime, float, float);
	return false;
}

void ATestEntity::OnBegin()
{
	startPos = GetLocation();
	timer = 0;
}

void ATestEntity::OnDestroy() 
{
	LOG("Ded");
}

void ATestEntity::OnTick(const float& deltaTime)
{
#ifdef BUILD_SERVER
	timer += deltaTime; 
	CallRPC_TwoParam(this, MoveTo, timer * 20.0f, 0);

	if (timer < 5.0f)
		return;
	timer = 0.0f;

	PrintTime(13.234f, 10.0f);
	CallRPC_TwoParam(this, PrintTime, 13.234f, 10.0f);
#endif
}

#ifdef BUILD_CLIENT
void ATestEntity::OnDraw(sf::RenderWindow* window, const float& deltaTime)
{
	sf::CircleShape shape(50);
	shape.setPosition(GetLocation());
	shape.setFillColor(sf::Color(255, 100, 50));
	window->draw(shape);

	//sf::CircleShape shape(50);
	//shape.setPosition(GetLocation());
	//shape.setFillColor(sf::Color(100, 255, 50));
	//window->draw(shape);
}
#endif

void ATestEntity::PrintTime(float time, float time2)
{
	LOG("Time, %f, %f", time, time2);
	//SetLocation(startPos + vec2(time, -time) * 20.0f);
}

void ATestEntity::ResetPosition()
{
	SetLocation(startPos);
}

void ATestEntity::MoveTo(float x, float y)
{
	SetLocation(startPos + sf::Vector2f(x, y));
}