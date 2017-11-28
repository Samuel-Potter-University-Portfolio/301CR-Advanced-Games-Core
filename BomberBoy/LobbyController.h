#pragma once
#include "Core\Core-Common.h"
#include "BPlayerController.h"


typedef std::map<uint16, uint8> PlayerVoteMap;
template<>
inline void Encode<PlayerVoteMap>(ByteBuffer& buffer, const PlayerVoteMap& data)
{
	Encode<uint16>(buffer, data.size());
	for (auto it : data)
	{
		Encode<uint16>(buffer, it.first);
		Encode<uint8>(buffer, it.second);
	}
}
template<>
inline bool Decode<PlayerVoteMap>(ByteBuffer& buffer, PlayerVoteMap& out, void* context)
{
	uint16 size;
	if (!Decode<uint16>(buffer, size))
		return false;
	out.clear();

	for (uint32 i = 0; i < size; ++i)
	{
		uint16 player;
		uint8 index;
		if (!Decode(buffer, player) || !Decode(buffer, index, context))
			return false;

		out[player] = index;
	}
	return true;
}



/**
* Level controller for when in the lobby screen
* Handles map voting and connected players
*/
class ALobbyController : public ALevelController
{
	CLASS_BODY()
public:
	static const std::vector<SubClassOf<LLevel>> s_supportedLevels;

private:
	float m_logicTimer;
	float m_totalTimer;
	bool bIsTimerActive = false;
	bool bIsLaunching = false;

	uint32 m_maxPlayers;
	uint32 m_requiredReadys;
	std::vector<class OBPlayerController*> m_players;

	// Store list of which player has voted for which map
	PlayerVoteMap m_mapVotes;

public:
	ALobbyController();

	virtual void OnBegin() override;
	virtual void OnTick(const float& deltaTime) override;

	virtual void OnPlayerConnect(OPlayerController* player, const bool& newConnection) override;
	virtual void OnPlayerDisconnect(OPlayerController* player) override;


	/**
	* Update which map this player has voted for
	* @param player			The player who is casting the vote
	* @param index			The index of the map
	*/
	inline void UpdateMapVote(OBPlayerController* player, const uint32& index) { m_mapVotes[player->GetNetworkID()] = index; }
private:

	/**
	* Net overrides
	*/
protected:
	virtual bool RegisterRPCs(const char* func, RPCInfo& outInfo) const override;
	virtual bool ExecuteRPC(uint16& id, ByteBuffer& params) override;

	virtual void RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode) override;
	virtual bool ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks) override;
	
private:

	/**
	* Getters & Setters
	*/
public:
	inline const bool& IsTimerActive() const { return bIsTimerActive; }
	inline const float& GetTimeUntilStart() const { return m_logicTimer; }

	uint32 GetMapVotes(const uint32& mapIndex) const;
};

