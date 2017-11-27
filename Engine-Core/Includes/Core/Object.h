#pragma once
#include "Common.h"
#include "NetSerializableBase.h"
#include "ManagedClass.h"


class Game;


/**
* Class representing a persistent object
* All children of this class should append an O to the beginning of their name
*/
class CORE_API OObject : public ManagedObject, public NetSerializableBase
{
	CLASS_BODY()
private:
	Game* m_game = nullptr;
	string m_name;

	bool bIsDestroyed = false;
	uint32 m_systemReferences = 0; // How many other sub-systems are current referencing this object

public:
	OObject();
	virtual ~OObject() {}

	/**
	* Callback for when object gets loaded into the game
	* @param game			The game that this game is loading into
	*/
	void OnGameLoaded(Game* game);
	/**
	* Callback for when this object is loaded into the game as an active object
	*/
	virtual void OnBegin() {}

	/**
	* Attempt to destroy this object
	* -Doesn't delete, just flags it for deletion
	*/
	static void Destroy(OObject* object);
	/**
	* Callback for when this object is being destroying
	*/
	virtual void OnDestroy() {}

	/**
	* Callback for when this object is created via a net call
	*/
	virtual void OnPostNetInitialize() {}



	inline void AddSystemReference() { ++m_systemReferences; }
	inline void RemoveSystemReference() { --m_systemReferences; }
	inline const uint32& GetRemainingSystemReferences() const { return m_systemReferences; }


	/**
	* Getters & Setters
	*/
public:
	inline void SetName(const string& name) { m_name = name; }
	inline const string& GetName() const { return m_name; }

	inline Game* GetGame() const { return m_game; }

	inline const bool& IsDestroyed() const { return bIsDestroyed; }
};


typedef OObject* OObjectPtr;
template<>
inline void Encode<OObjectPtr>(ByteBuffer& buffer, const OObjectPtr& data)
{
	Encode(buffer, data->GetNetworkID());
}

template<>
bool CORE_API Decode<OObjectPtr>(ByteBuffer& buffer, OObjectPtr& out, void* context);