#include "Includes\Core\Actor.h"
#include "Includes\Core\Level.h"


CLASS_SOURCE(AActor, CORE_API)
uint32 AActor::s_instanceCounter = 0;


AActor::AActor() :
	m_instanceId(s_instanceCounter++)
{
	bIsTickable = false;
	m_drawingLayer = 0;
}

void AActor::OnLevelLoaded(LLevel* level)
{
	m_level = level;
	OnGameLoaded(level->GetGame());
}