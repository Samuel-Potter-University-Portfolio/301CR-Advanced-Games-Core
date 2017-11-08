#include "Core/Common.h"
#include "Core/Engine.h"
#include "Core/Game.h"


#include "MainLevel.h"

#include "Core\Camera.h"
#include "TestEntity.h"


#include "Core\NetSerializableBase.h"

class TESTCLASS : public NetSerializableBase
{
public:
	TESTCLASS();
	void TestFunc();
	void TestFunc2(int no);


	void ComplexFunction(string message, int repeatTimes);


public:
	virtual bool FetchRPCIndex(const char* funcName, uint16& outID) const;
	virtual bool ExecuteRPC(uint16& id, ByteBuffer& params);
};


TESTCLASS::TESTCLASS() 
{
	bNetSynced = true;
}

void TESTCLASS::ComplexFunction(string message, int repeatTimes) 
{
	LOG("Complex Function start");
	for (int i = 0; i < repeatTimes; i++)
	{
		LOG("\t%s", message.c_str());
	}

	LOG("Complex Function end");
}

void TESTCLASS::TestFunc()
{
	LOG("This is test func 1");
}
void TESTCLASS::TestFunc2(int no)
{
	LOG("This is test func 2 with %i", no);
}

bool TESTCLASS::FetchRPCIndex(const char* funcName, uint16& outID) const 
{
	RPC_INDEX_HEADER(funcName, outID);
	RPC_INDEX(TestFunc);
	RPC_INDEX(TestFunc2);
	RPC_INDEX(ComplexFunction);
	return false;
}

bool TESTCLASS::ExecuteRPC(uint16& id, ByteBuffer& params) 
{
	RPC_EXEC_HEADER(id, params);
	RPC_EXEC(TestFunc);
	RPC_EXEC_OneParam(TestFunc2, int);
	RPC_EXEC_TwoParam(ComplexFunction, string, int);
	return false;
}



static inline int entry(std::vector<string>& args)
{
	TESTCLASS tc;
	CallRPC(UDP, RPCTarget::GlobalBroadcast, &tc, TestFunc);
	CallRPC_OneParam(UDP, RPCTarget::GlobalBroadcast, &tc, TestFunc2, 34);
	CallRPC_TwoParam(UDP, RPCTarget::GlobalBroadcast, &tc, ComplexFunction, (const char*)"Hello There", 4);

	ByteBuffer& buffer = tc.m_UdpCallQueue;
	buffer.Flip();

	uint16 funcIndex;
	uint8 rawFuncTarget;
	uint16 paramCount;

	while (buffer.Size() != 0)
	{
		Decode(buffer, funcIndex);
		Decode(buffer, rawFuncTarget);
		Decode(buffer, paramCount);
		RPCTarget target = (RPCTarget)rawFuncTarget;
		tc.ExecuteRPC(funcIndex, buffer);
	}

	/*
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
	*/
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