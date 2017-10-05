#pragma once
#include "Level.h"


class TestLevel : public Level
{
public:
	TestLevel();

	/**
	* Called when this level comes into usage
	*/
	virtual void BuildLevel();
	/**
	* Called when this level is about to go out of usage
	*/
	virtual void DestroyLevel();
};

