#include "MainLevel.h"


MainLevel::MainLevel() : Level("Main")
{

}

void MainLevel::BuildLevel() 
{
	SpawnEntity("TestEntity");
}
