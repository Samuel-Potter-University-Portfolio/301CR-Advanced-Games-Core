#include "Game.h"


GameInfo::GameInfo(std::vector<string>& args)
{
	// TODO - Parse any command line args
}

Game::Game(GameInfo* info)
{
	m_name = info->name;
}

Game::~Game()
{
}

void Game::HookEngine(Engine* engine)
{
	m_engine = engine;
}

void Game::MainUpdate(const float& deltaTime) 
{
	static float timer = 0;
	timer += deltaTime;

	if (timer >= 1.0f)
	{
		timer -= 1.0f;
		LOG("Main sec");
	}
}

#ifdef BUILD_CLIENT
void Game::DisplayUpdate(const float& deltaTime) 
{
	static float timer = 0;
	timer += deltaTime;

	if (timer >= 1.0f)
	{
		timer -= 1.0f;
		LOG("Display sec");
	}
}
#endif