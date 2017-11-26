#pragma once
#include "Core\Core-Common.h"


class ALobbyController : public ALevelController
{
	CLASS_BODY()
public:
	ALobbyController();

	virtual void OnPlayerConnect(OPlayerController* player, const bool& newConnection) override;
	virtual void OnPlayerDisconnect(OPlayerController* player) override;
};

