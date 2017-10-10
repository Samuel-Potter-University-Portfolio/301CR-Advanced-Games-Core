#pragma once
#include "Common.h"
#include "Game.h"
#include "Engine.h"

namespace EngineCore
{
	class Entity;


	/**
	* Represents a collections of entities which all form a level
	*/
	class CORE_API Level
	{
	private:
		string m_name;
		Game* m_game;
		std::vector<Entity*> m_entities;

	public:
		Level(string name);
		~Level();

		/**
		* Callback for when the game registers this level
		* @param game		Game in question
		*/
		void HookGame(Game* game);

		/**
		* Called when this level comes into usage
		*/
		virtual void BuildLevel() = 0;
		/**
		* Called when this level is about to go out of usage
		*/
		virtual void DestroyLevel();

		/**
		* Called when an entity should be added to a level
		* @param entity			Desired entity
		*/
		void AddEntity(Entity* entity);

		/**
		* Getters and setters
		*/
	public:
		inline string GetName() const { return m_name; }
		inline Game* GetGame() const { return m_game; }

		inline std::vector<Entity*> GetEntities() const { return m_entities; }
	};
}