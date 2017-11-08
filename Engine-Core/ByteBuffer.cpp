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