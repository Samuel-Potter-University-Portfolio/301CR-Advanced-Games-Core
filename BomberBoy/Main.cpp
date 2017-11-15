#include "Core/Common.h"
#include "Core/Engine.h"
#include "Core/Game.h"


#include "MainLevel.h"

#include "Core\Camera.h"
#include "TestEntity.h"


static inline int entry(std::vector<string>& args)
{
	LOG("Discovered %i cmd arguments", args.size());
	for (string& str : args)
		LOG("\t'%s'", str.c_str())


	// Setup engine
	Engine engine(args);

	// Setup game
	Game game("Bomber Boy", Version(0,1,0));
	{
		// Add levels
		game.RegisterClass(LMainLevel::StaticClass());

		game.defaultLevel = LMainLevel::StaticClass();
		game.defaultNetLevel = LMainLevel::StaticClass();

		// Register actors
		game.RegisterClass(ACamera::StaticClass());
		game.RegisterClass(ATestEntity::StaticClass());

		// Register assets
	}

	engine.Launch(&game);
	return 0;
}




/**
* Using appropriate entry point based on build mode
* and converting launch args into vector
*/


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
	system("pause");
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