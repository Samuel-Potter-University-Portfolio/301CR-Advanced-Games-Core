#pragma once
#include "Game.h"
#include "Engine.h"


class Level
{
private:
	string m_name;
	Game* m_game;

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
	* Getters and setters
	*/
public:
	inline string GetName() const { return m_name; }
	inline Game* GetGame() const { return m_game; }
};

