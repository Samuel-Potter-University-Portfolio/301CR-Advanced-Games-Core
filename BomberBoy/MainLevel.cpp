#include "MainLevel.h"

#include "Core\Camera.h"
#include "TestEntity.h"


MainLevel::MainLevel() : Level("Main")
{


}

void MainLevel::BuildLevel() 
{
	SpawnEntity<Camera>();
	SpawnEntity<TestEntity>();
	TestEntity* e = SpawnEntity<TestEntity>(1);
	e->SetLocation(sf::Vector2f(0, 100));
}
