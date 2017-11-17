#pragma once
#include "Common.h"

#include <vector>
#include <SFML\Graphics.hpp>


/**
* Holds a series of textures (In order) and lets them loop through at a set framerate
* -NOTE: Doesn't manage memory for given textures
*/
class CORE_API AnimationSheet
{
private:
	float m_timer = 0;
	float m_frameDuration = 0.2f;

	uint32 m_currentFrame = 0;
	std::vector<const sf::Texture*> m_timeline;

public:
	/**
	* Updates the animation
	* @param deltaTime			Time since last frame (In seconds)
	*/
	void UpdateAnimation(const float& deltaTime);
	

	/**
	* Getters & Setters
	*/
public:
	/**
	* Adds frame to this animation sheet
	* @param texture		The frame to add
	*/
	inline void AddFrame(const sf::Texture* texture) { m_timeline.emplace_back(texture); }
	inline const sf::Texture* GetCurrentFrame() const { return m_timeline[m_currentFrame]; }

	inline void SetFrameDuration(const float& value) { m_frameDuration = value; }
	inline const float& GetFrameDuration() const { return m_frameDuration; }
	inline float GetTotalDuration() const { return m_frameDuration * m_timeline.size(); }


};

