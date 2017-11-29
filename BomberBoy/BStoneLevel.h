#pragma once
#include "BGameLevelBase.h"

class LBStoneLevel : public LBGameLevelBase
{
	CLASS_BODY()
public:
	/** Callback for when this arena should be setup */
	virtual void BuildArena(ABLevelArena* arena);
};

