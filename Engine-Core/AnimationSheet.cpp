#include "Includes\Core\AnimationSheet.h"



void AnimationSheet::UpdateAnimation(const float& deltaTime) 
{
	m_timer += deltaTime;

	const float totalDuration = GetTotalDuration();
	if (m_timer > totalDuration)
		m_timer -= totalDuration;

	m_currentFrame = (uint32)(m_timer/ m_frameDuration);
}