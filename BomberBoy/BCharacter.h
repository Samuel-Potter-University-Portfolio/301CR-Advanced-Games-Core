#pragma once
#include "Core\Core-Common.h"
#include "BTileableActor.h"

#include "Core\Camera.h"
#include "BBomb.h"


/**
* Represents a colour that players can be
*/
struct CharacterColour 
{
	const string	name;
	const Colour	colour;

	CharacterColour(const string& name, const Colour& colour)
		: name(name), colour(colour) {}
};



/**
* Represents player characters used in BomberBoy
*/
class ABCharacter : public ABTileableActor
{
	CLASS_BODY()
public:
	static const std::vector<CharacterColour> s_supportedColours;
	static const uint32 s_maxBombCount;

private:
	const AnimationSheet* m_animUp = nullptr;
	const AnimationSheet* m_animDown = nullptr;
	const AnimationSheet* m_animLeft = nullptr;
	const AnimationSheet* m_animRight = nullptr;

	ACamera* m_camera;
	const vec2 m_drawSize;
	const vec2 m_drawOffset;


	KeyBinding m_upKey;
	KeyBinding m_downKey;
	KeyBinding m_leftKey;
	KeyBinding m_rightKey;
	KeyBinding m_bombKey;


	/// Pool bombs
	std::vector<ABBomb*> m_bombs;
	/// Get a bomb to use
	inline ABBomb* GetNewBomb() const 
	{ 
		for (ABBomb* bomb : m_bombs)
			if (!bomb->IsActive())
				return bomb;
		return nullptr;
	}

public:
	ABCharacter();

	virtual void OnBegin() override;
	virtual void OnDestroy() override;

	/**
	* Registers the needed assets for ABCharacter
	* @param game			Where to register the assets
	*/
	static void RegisterAssets(Game* game);


	virtual void OnTick(const float& deltaTime) override;
#ifdef BUILD_CLIENT
	virtual void OnDraw(sf::RenderWindow* window, const float& deltaTime) override;
#endif

	/**
	* Attempt to place a bomb at this player's feet
	* @param tile		The tile to place it on
	*/
	void PlaceBomb(const ivec2& tile);

	/**
	* Net overrides
	*/
protected:
	virtual bool RegisterRPCs(const char* func, RPCInfo& outInfo) const override;
	virtual bool ExecuteRPC(uint16& id, ByteBuffer& params) override;

	//virtual void RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode) override;
	//virtual bool ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks) override;


};


typedef ABCharacter* ABCharacterPtr;
template<>
inline void Encode<ABCharacter*>(ByteBuffer& buffer, const ABCharacterPtr& data)
{
	Encode(buffer, data->GetNetworkID());
}

template<>
inline bool Decode<ABCharacter*>(ByteBuffer& buffer, ABCharacterPtr& out, void* context)
{
	out = nullptr;
	uint16 id;
	if (!Decode<uint16>(buffer, id))
		return false;

	Game* game = (Game*)context;
	if (game == nullptr)
	{
		LOG_ERROR("Cannot decode 'ABCharacter*' without Game* as context");
		return true; // Decode was fine, just invalid context used
	}

	LLevel* level = game->GetCurrentLevel();
	if (level == nullptr)
		return true; // Decode was fine, but there was no level active
	else
	{
		out = dynamic_cast<ABCharacter*>(level->GetActorByNetID(id));
		return true;
	}
}