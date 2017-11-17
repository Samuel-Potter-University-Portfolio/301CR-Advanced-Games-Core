#pragma once
#include "Core\Actor.h"


class ATestEntity : public AActor
{
	CLASS_BODY()
public:
	ATestEntity();


protected:
	virtual bool RegisterRPCs(const char* func, RPCInfo& outInfo) const override;
	virtual bool ExecuteRPC(uint16& id, ByteBuffer& params) override;
	
	virtual void RegisterSyncVars(SyncVarQueue& outQueue, const SocketType& socketType, uint16& index, uint32& trackIndex) override;
	virtual bool ExecuteSyncVar(uint16& id, ByteBuffer& value) override;

protected:
	virtual void OnBegin();
	virtual void OnDestroy() override;

	/**
	* Callback for level first tick
	* @param deltaTime		Time since last tick in seconds
	*/
	virtual void OnTick(const float& deltaTime);

	sf::Vector2f startPos;
	float timer;
	int clampedTimer;
	string testString;
	void ResetPosition();
	void MoveTo(float x, float y);
	void PrintTime(float time, float time2);

	void OnClampedTimerChange();
	void OnTestStringChange();

#ifdef BUILD_CLIENT
	/**
	* Called when this entity should be drawn to the screen
	* @param window			The window to draw to
	* @param deltaTime		Time since last draw in seconds)
	*/
	virtual void OnDraw(sf::RenderWindow* window, const float& deltaTime) override;
#endif
};

