#pragma once
#include "Includes\Core\Engine.h"
#include "Common.h"


namespace PotterEngine
{	
	ref class Game;


	public ref struct EngineInfo
	{
	public:
		uint32 windowWidth = 800;
		uint32 windowHeight = 600;

		EngineInfo(array<System::String^>^ args);
	};


	public ref class Engine
	{
	internal:
		EngineCore::Engine* m_engine;

	public:
		Engine(EngineInfo^ info);
		~Engine();
		

		void Launch(Game^ game);
		inline void Close() { m_engine->Close(); }
	};
};