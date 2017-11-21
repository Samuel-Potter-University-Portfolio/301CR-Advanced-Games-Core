#pragma once
#include "Core\Core-Common.h"
#include "BTileableActor.h"


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

private:
	const AnimationSheet* m_animUp = nullptr;
	const AnimationSheet* m_animDown = nullptr;
	const AnimationSheet* m_animLeft = nullptr;
	const AnimationSheet* m_animRight = nullptr;

	const vec2 m_drawSize;
	const vec2 m_drawOffset;


	KeyBinding m_upKey;
	KeyBinding m_downKey;
	KeyBinding m_leftKey;
	KeyBinding m_rightKey;

public:
	ABCharacter();

	virtual void OnBegin() override;
	//virtual void OnDestroy() override;

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
	* Net overrides
	*/
protected:
	//virtual bool RegisterRPCs(const char* func, RPCInfo& outInfo) const override;
	//virtual bool ExecuteRPC(uint16& id, ByteBuffer& params) override;

	//virtual void RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode) override;
	//virtual bool ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks) override;


};

