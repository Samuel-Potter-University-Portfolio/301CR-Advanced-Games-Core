#include "Includes\Core\ByteBuffer.h"
#include <memory>


void ByteBuffer::Push(const uint8* b, uint32 count)
{
	const uint32 startSize = m_data.size();
	m_data.resize(startSize + count);
	std::memcpy(m_data.data() + startSize, b, count);
}

void ByteBuffer::Flip() 
{
	std::reverse(m_data.begin(), m_data.end());
}

void ByteBuffer::PopBuffer(ByteBuffer& target, uint32 count) 
{
	// Make sure count isn't too big
	count = m_data.size() < count ? m_data.size() : count;

	target.Push(m_data.data() + m_data.size() - count, count);
	m_data.resize(m_data.size() - count);
}