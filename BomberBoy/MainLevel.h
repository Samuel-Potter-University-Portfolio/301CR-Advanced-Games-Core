#pragma once
#include "Core\Core-Common.h"


class LMainLevel : public LLevel
{
	CLASS_BODY()
public:
	LMainLevel();
	
	/**
	* Called when this level comes into usage
	*/
	virtual void OnBuildLevel() override;
};

