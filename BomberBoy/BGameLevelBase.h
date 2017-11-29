#pragma once
#include "Core\Core-Common.h"

#include "BLevelArena.h"


/**
* Base level for all gamemodes
*/
class LBGameLevelBase : public LLevel
{
	CLASS_BODY()	

public:
	LBGameLevelBase();

	/** Called when this level comes into usage	*/
	virtual void OnBuildLevel() override;

	/** Callback for when this arena should be setup */
	virtual void BuildArena(ABLevelArena* arena) {}
};

