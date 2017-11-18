#pragma once
#include "Core\Core-Common.h"


class ATestEntity : public AActor
{
	CLASS_BODY()
private:
	AnimationSheet m_testAnimation;

public:
	ATestEntity();

protected:
	virtual bool RegisterRPCs(const char* func, RPCInfo& outInfo) const override;
	virtual bool ExecuteRPC(uint16& id, ByteBuffer& params) override;
	
	virtual void RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex, const bool& forceEncode) override;
	virtual bool ExecuteSyncVar(uint16& id, ByteBuffer& value, const bool& skipCallbacks) override;

protected:
	virtual void OnBegin();
	virtual void OnDestroy() override;


	sf::Vector2f startPos;
	float timer;
	int clampedTimer;
	string testString;
	void ResetPosition();
	void MoveTo(float x, float y);
	void PrintTime(float time, float time2);

	void OnClampedTimerChange();
	void OnTestStringChange();

	/**
	* Callback for level first tick
	* @param deltaTime		Time since last tick in seconds
	*/
	virtual void OnTick(const float& deltaTime);
#ifdef BUILD_CLIENT
	/**
	* Called when this entity should be drawn to the screen
	* @param window			The window to draw to
	* @param deltaTime		Time since last draw in seconds)
	*/
	virtual void OnDraw(sf::RenderWindow* window, const float& deltaTime) override;
#endif
};

