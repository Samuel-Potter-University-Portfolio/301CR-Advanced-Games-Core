#include "Includes\Core\AnimationSheet.h"



void AnimationSheet::UpdateAnimation(const float& deltaTime) 
{
	m_timer += deltaTime;

	const float totalDuration = GetTotalDuration();
	if (m_timer > totalDuration)
		m_timer -= totalDuration;

	const uint32 frame = (uint32)(m_timer/ m_frameDuration);
	m_currentFrame = frame % m_timeline.size();
}