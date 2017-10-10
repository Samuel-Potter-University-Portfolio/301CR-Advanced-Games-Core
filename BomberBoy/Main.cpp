#include "Core/Common.h"
#include "Core/Engine.h"
#include "Core/Game.h"
#include <SFML/Graphics.hpp>


inline int entry(std::vector<string>& args)
{
	LOG("Discovered %i cmd arguments", args.size());

	for (string& str : args)
	{
		LOG("\t-'%s'", str.c_str());
	}

	// Setup engine
	EngineInfo engineInfo(args);
	Engine engine(&engineInfo);

	// Setup game
	Game game("Untitled Game");


	// Register levels
	game.SetDefaultLevelName("Test");
	//game.RegisterLevel(new TestLevel);

	engine.Launch(&game);
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
	int o = entry(args);

	// Wait whilst in debug to read log
#ifdef BUILD_DEBUG
	while (true) {}
#endif
	return o;
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