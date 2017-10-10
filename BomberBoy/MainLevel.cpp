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
}
