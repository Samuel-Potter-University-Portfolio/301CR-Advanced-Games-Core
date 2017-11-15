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

bool ATestEntity::FetchRPCIndex(const char* funcName, uint16& outID) const
{
	RPC_INDEX_HEADER(funcName, outID);
	RPC_INDEX(ResetPosition);
	RPC_INDEX(MoveTo);
	RPC_INDEX(PrintTime);
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
	
	//if (timer < 3.0f)
	//	return;

	//timer = 0.0f;


	/*
	LOG("m netid %i", GetNetworkID());
	LOG("m ownerid %i", GetNetworkOwnerID());

	std::vector<OPlayerController*> controllers = GetLevel()->GetGame()->GetActiveObjects<OPlayerController>();
	LOG("%i players", controllers.size());
	for (auto ply : controllers)
	{
		LOG("\t id:%i %i", ply->GetNetworkOwnerID(), ply->IsNetOwner());
	}
	*/


	//if (timer >= 10.0f)
	//{
	//	timer = 0.0f;
	//	CallRPC(TCP, RPCTarget::Owner, this, ResetPosition);
	//	return;
	//}

	CallRPC_TwoParam(UDP, RPCTarget::ClientBroadcast, this, PrintTime, 13.234f, 10.0f);

	//CallRPC_TwoParam(UDP, RPCTarget::GlobalBroadcast, this, PrintTime, timer, 10.0f - timer);

	//sf::Vector2f dl = GetLocation() + sf::Vector2f(30, 0) * deltaTime;
	//CallRPC_TwoParam(TCP, RPCTarget::Owner, this, MoveTo, dl.x, dl.y);
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
	LOG("Hello, %f, %f", time, time2);
	//SetLocation(startPos + vec2(time, -time) * 20.0f);
}

void ATestEntity::ResetPosition()
{
	SetLocation(startPos);
}

void ATestEntity::MoveTo(float x, float y)
{
	SetLocation(sf::Vector2f(x, y));
}