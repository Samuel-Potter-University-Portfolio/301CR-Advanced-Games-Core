#pragma once
#include "Core\Core-Common.h"


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
* Used for syncing animation and sending to server
*/
enum class CharacterDirection : uint8
{
	Up, Down, Left, Right
};

template<>
inline void Encode<CharacterDirection>(ByteBuffer& buffer, const CharacterDirection& data)
{
	Encode<uint8>(buffer, (uint8)data);
}
template<>
inline bool Decode<CharacterDirection>(ByteBuffer& buffer, CharacterDirection& out, void* context)
{
	uint8 raw;
	if (!Decode<uint8>(buffer, raw))
		return false;
	out = (CharacterDirection)raw;
	return true;
}



/**
* Represents player characters used in BomberBoy
*/
class ABomberCharacter : public AActor
{
	CLASS_BODY()
public:
	static const std::vector<CharacterColour> s_supportedColours;

private:
	const AnimationSheet* m_animUp = nullptr;
	const AnimationSheet* m_animDown = nullptr;
	const AnimationSheet* m_animLeft = nullptr;
	const AnimationSheet* m_animRight = nullptr;

	const vec2 m_drawSize;
	const vec2 m_drawOffset;

	CharacterDirection m_direction;
	CharacterDirection m_netDirection;

	KeyBinding m_upKey;
	KeyBinding m_downKey;
	KeyBinding m_leftKey;
	KeyBinding m_rightKey;

public:
	ABomberCharacter();

	virtual void OnBegin() override;
	//virtual void OnDestroy() override;

	/**
	* Registers the needed assets for ABomberCharacter
	* @param game			Where to register the assets
	*/
	static void RegisterAssets(Game* game);


	virtual void OnTick(const float& deltaTime) override;
#ifdef BUILD_CLIENT
	virtual void OnDraw(sf::RenderWindow* window, const float& deltaTime) override;
#endif


	/**
	* Net overrides
	*/
protected:
	virtual bool RegisterRPCs(const char* func, RPCInfo& outInfo) const override;
	virtual bool ExecuteRPC(uint16& id, ByteBuffer& params) override;

	virtual void RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode) override;
	virtual bool ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks) override;

	/** Sends the players direction to the server */
	void UpdateNetDirection(const CharacterDirection& direction);
};

