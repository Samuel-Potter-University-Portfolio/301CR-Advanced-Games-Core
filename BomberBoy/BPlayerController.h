#pragma once
#include "Core\Core-Common.h"
#include "BCharacter.h"


/**
* Player controller that is used for all BomberBoy gamemodes
*/
class OBPlayerController : public OPlayerController
{
	CLASS_BODY()
	friend class ABLevelControllerBase;
private:
	ABCharacter* m_character = nullptr;

public:
	OBPlayerController();
};

