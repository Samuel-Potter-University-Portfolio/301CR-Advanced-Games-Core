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

	int64 m_matchStartEpoch;
	uint32 m_roundCounter = 0;
	const uint32 m_roundWinAmount = 5;

	std::vector<OBPlayerController*> m_activePlayers;

public:
	ABMatchController();

	virtual void OnBegin() override;
	virtual void OnTick(const float& deltaTime) override;

	virtual void OnPlayerConnect(OPlayerController* player, const bool& newConnection) override;
	virtual void OnPlayerDisconnect(OPlayerController* player) override;


	/**
	* Callback to decide what to do when a player gets exploded by someone
	* @param victim				The person who is in the explosion
	* @param killer				The person who owned the explosion
	*/
	void OnPlayerExploded(ABCharacter* victim, ABCharacter* killer);

	/**
	* Send a message to all clients
	* @param message			The message to send
	*/
	void SendChatMessage(const string& message);


	/**
	* Net overrides
	*/
protected:
	virtual bool RegisterRPCs(const char* func, RPCInfo& outInfo) const override;
	virtual bool ExecuteRPC(uint16& id, ByteBuffer& params) override;

	//virtual void RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode) override;
	//virtual bool ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks) override;


	/**
	* Getters & Setters
	*/
public:
	inline const std::vector<OBPlayerController*> GetActivePlayers() const { return m_activePlayers; }

	inline const MatchState& GetMatchState() const { return m_currentState; }
	inline const float& GetStateTimer() const { return m_stateTimer; }
};

