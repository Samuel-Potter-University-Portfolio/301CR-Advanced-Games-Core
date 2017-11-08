#pragma once
#include "Common.h"
#include "Types.h"
#include "ClassFactory.h"
#include "NetSerializableBase.h"
#include <SFML/Graphics.hpp>


class Level;


/**
* Represents anything which exists in a level
*/
class CORE_API Entity : public NetSerializableBase
{
private:
	friend class Level;
	bool bWasLoadedWithLevel = false;
	uint32 m_typeId;
	uint32 m_instanceId;

	string m_name;
	Level* m_level;

	bool bHasStarted = false;
	uint8 m_sortingLayer;

	sf::Vector2f location;

public:
	Entity();
	virtual ~Entity();

	/**
	* Callback for when this entity is spawned into this level
	* @param level		Level this entity exists in
	*/
	void HandleSpawn(Level* level);

	/**
	* Callback for level main update
	* @param deltaTime		Time since last tick in seconds
	*/
	void HandleMainUpdate(const float& deltaTime);

#ifdef BUILD_CLIENT
	/**
	* Called when this entity should be drawn to the screen
	* @param window			The window to draw to
	* @param deltaTime		Time since last draw in seconds)
	*/
	virtual void Draw(sf::RenderWindow* window, const float& deltaTime);
#endif

protected:
	/**
	* Callback for entity's level first tick
	*/
	virtual void OnBegin() {}

	/**
	* Callback for level first tick
	* @param deltaTime		Time since last tick in seconds
	*/
	virtual void OnTick(const float& deltaTime) {}

	/**
	* Getters & Setters
	*/
public:
	inline string GetName() const { return m_name; }
	inline void SetName(string name) { m_name = name; }

	inline Level* GetLevel() const { return m_level; }

	inline uint8 GetSortingLayer() const { return m_sortingLayer; }
	inline void SetSortingLayer(uint8 layer) { m_sortingLayer = layer; }

	inline sf::Vector2f GetLocation() const { return location; }
	inline void SetLocation(sf::Vector2f loc) { location = loc; }

	inline uint32 GetTypeID() const { return m_typeId; }
	inline uint32 GetInstanceID() const { return m_instanceId; }

	inline const bool& WasLoadedWithLevel() const { return bWasLoadedWithLevel; }
};



/**
* Factory used for creating entities
*/
template<class Type>
class CORE_API EntityFactory : public ChildFactory<Entity, Type> 
{};