#include "Engine.h"
#include "Game.h"


using namespace PotterEngine;


EngineInfo::EngineInfo(array<System::String^>^ args) 
{
	// TODO - Parse
}


Engine::Engine(EngineInfo^ info)
{
	EngineCore::EngineInfo init;
	init.windowWidth = info->windowWidth;
	init.windowHeight = info->windowHeight;

	m_engine = new EngineCore::Engine(&init);
}

Engine::~Engine()
{
	delete m_engine;
}

void Engine::Launch(Game^ game) 
{
	m_engine->Launch(game->m_game);
}