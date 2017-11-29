#pragma once
#include "Core\Core-Common.h"
#include "BPlayerController.h"


/**
* Level controller base for all BomberBoy gamemodes
*/
class ABMatchController : public ALevelController
{
	CLASS_BODY()
public:
	enum MatchState : uint8
	{
		InActive,
		StartingRound,
		InRound,
		EndOfRound,
		EndOfMatch
	};

private:
	/// Reuseable timer for whatever state the match is in
	float m_stateTimer;
	MatchState m_currentState;

	std::vector<OBPlayerController*> m_activePlayers;

public:
	ABMatchController();

	virtual void OnBegin() override;
	virtual void OnTick(const float& deltaTime) override;

	virtual void OnPlayerConnect(OPlayerController* player, const bool& newConnection) override;
	virtual void OnPlayerDisconnect(OPlayerController* player) override;


	/**
	* Getters & Setters
	*/
public:
	inline const std::vector<OBPlayerController*> GetActivePlayers() const { return m_activePlayers; }

	inline const MatchState& GetMatchState() const { return m_currentState; }
	inline const float& GetStateTimer() const { return m_stateTimer; }
};

