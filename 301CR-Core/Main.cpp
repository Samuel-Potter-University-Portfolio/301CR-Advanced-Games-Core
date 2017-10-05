#include "Logger.h"
#include "Engine.h"
#include "Game.h"
#include <SFML/Graphics.hpp>


inline int entry(std::vector<string>& args)
{
	LOG("Discovered %i cmd arguments", args.size());

	for (string& str : args)
	{
		LOG("\t-'%s'", str.c_str());
	}


	EngineInfo engineInfo(args);
	GameInfo gameInfo(args);
	gameInfo.name = "Untitled Game";

	Engine* engine = new Engine(&engineInfo);
	Game* game = new Game(&gameInfo);

	engine->Launch(game);

	delete game;
	delete engine;


	// Wait whilst in debug to read log
#ifdef BUILD_DEBUG
	while (true) {}
#endif
	return 0;
}

/**
* Convert cmd arguments into more usable string array
*/
std::vector<string> GetArgs(int argc, wchar_t** argv)
{
	std::vector<string> out;
	out.reserve(argc);

	for (int i = 0; i < argc; ++i)
	{
		std::wstring ws(argv[i]);
		out.emplace_back(ws.begin(), ws.end());
	}

	return out;
}



#if BUILD_DEBUG || BUILD_SERVER

// Show Cmd for only server or debug builds
int wmain(int argc, wchar_t** argv)
{
	std::vector<string> args = GetArgs(argc, argv);
	return entry(args);
}

#else

#include <Windows.h>

int CALLBACK WinMain(
	HINSTANCE	hInstance,
	HINSTANCE	hPrevInstance,
	LPSTR		lpCmdLine,
	int			nCmdShow
	) 
{
	// Fetch cmd args
	LPWSTR* argv;
	int argc;
	argv = CommandLineToArgvW(GetCommandLineW(), &argc);

	std::vector<string> args = GetArgs(argc, argv);
	LocalFree(argv);

	return entry(args);
}

#endif