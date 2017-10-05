#include "Logger.h"
#include "Engine.h"
#include <SFML/Graphics.hpp>


int main(int argc, char** argv)
{
	EngineInfo info(argc, argv);

	Engine* engine = new Engine(&info);
	engine->Launch();

	delete engine;
	return 0;
}