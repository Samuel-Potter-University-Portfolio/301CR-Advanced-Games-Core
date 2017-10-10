#pragma once
#include "Includes\Core\Game.h"
#include "Common.h"


namespace PotterEngine
{
	public ref class Game
	{
	internal:
		EngineCore::Game* m_game;

	public:
		Game(System::String^ name);
		~Game();

		/**
		* Getters & Setters
		*/
	public:
		//inline Engine* GetEngine() { return m_engine; m_engine; }

		property System::String^ name
		{
			inline System::String^ get()
			{
				return Convert<System::String^>(m_game->GetName());
			}
		}

		property System::String^ defaultLevelName 
		{
			inline System::String^ get() 
			{
				return Convert<System::String^>(m_game->GetDefaultLevelName());
			}
			inline void set(System::String^ str)
			{
				m_game->SetDefaultLevelName(Convert<std::string>(str));
			}
		}
	};
};