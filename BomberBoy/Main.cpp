#include "Core/Common.h"
#include "Core/Engine.h"
#include "Core/Game.h"


#include "MainLevel.h"

#include "Core\Camera.h"
#include "TestEntity.h"


#include "Core\NetSocketTcp.h"
#include "Core\NetSocketUdp.h"
#include <sstream>


static inline int entry(std::vector<string>& args)
{
	/*
	NetSocketUdp sock;

	if (sock.Connect("localhost", 20010)) 
	{
		LOG("CONNECT");

		std::stringstream ss;
		for (int i = 0; i < 100; ++i)
			ss << i << ' ';

		const uint8* data = (const uint8*)ss.str().c_str();
		if (sock.Send(data, ss.str().length()))
		{
			LOG("YES SEND");
		}
		else
		{
			LOG("NO SEND");
		}
	}
	else
	{
		LOG("NO CONNECT");
	}
	/*/
	//*
	NetSocketUdp listener;
	
	if (listener.Listen(20010))
	{
		LOG("Listening on %s:%i", listener.GetAddress().toString().c_str(), listener.GetPort());
		std::vector<RawNetPacket> input;

		while (true)
		{
			input.clear();

			if (listener.Poll(input))
			{
				for(RawNetPacket& packet : input)
				{
					LOG("Received (%s:%i) %i bytes", packet.sourceAddress.toString().c_str(), packet.sourcePort, packet.dataCount);
					LOG("\t%s", packet.data);
				}
				listener.Send((const uint8*)"OK", 3);
			}
		}
	}
	//*/
	return 0;












	LOG("Discovered %i cmd arguments", args.size());
	for (string& str : args)
		LOG("\t'%s'", str.c_str())


	// Setup engine
	EngineInfo engineInfo(args);
	Engine engine(&engineInfo);

	// Setup game
	Game game("Bomber Boy");
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