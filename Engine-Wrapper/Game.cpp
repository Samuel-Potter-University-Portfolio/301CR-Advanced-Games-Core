#include "Game.h"


using namespace PotterEngine;


Game::Game(System::String^ name)
{
	m_game = new EngineCore::Game(Convert<std::string>(name));
}

Game::~Game() 
{
	delete m_game;
}