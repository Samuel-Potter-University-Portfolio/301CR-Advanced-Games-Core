#pragma once
#include "Core\Level.h"

class MainLevel : public Level
{
public:
	MainLevel();
	
	/**
	* Called when this level comes into usage
	*/
	virtual void BuildLevel();
};

