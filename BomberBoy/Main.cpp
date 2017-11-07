#include "Core/Common.h"
#include "Core/Engine.h"
#include "Core/Game.h"


#include "MainLevel.h"

#include "Core\Camera.h"
#include "TestEntity.h"

#include "Core\Encoding.h"
#include "Core\Version.h"


static inline int entry(std::vector<string>& args)
{
	ByteBuffer bb;
	string str = "Another test string";

	Encode<int8>(bb, 12);
	Encode<int8>(bb, 34);
	Encode<const char*>(bb, "Hello World how are you today?");
	Encode<string>(bb, str);
	Encode<float>(bb, 234.23f);
	Encode<int32>(bb, 21474);
	Encode<Version>(bb, Version(23, 145, 123));
	
	int outA;
	float outB;
	string outStrA;
	string outStrB;
	int8 outC;
	int8 outD;
	Version outV;
	bb.Flip();

	Decode<int8>(bb, outD);
	Decode<int8>(bb, outC);
	Decode<string>(bb, outStrA);
	Decode<string>(bb, outStrB);
	Decode<float>(bb, outB);
	Decode<int32>(bb, outA);
	Decode<Version>(bb, outV);

	LOG("Discovered %i cmd arguments", args.size());
	for (string& str : args)
		LOG("\t'%s'", str.c_str())


	// Setup engine
	EngineInfo engineInfo(args);
	Engine engine(&engineInfo);

	// Setup game
	Game game("Bomber Boy", Version(0,1,0));
	{
		// Add levels
		game.SetDefaultLevelName("Main");
		game.RegisterLevel(new MainLevel);

		// Register entities
		game.RegisterEntity(new EntityFactory<Camera>());
		game.RegisterEntity(new EntityFactory<TestEntity>());

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