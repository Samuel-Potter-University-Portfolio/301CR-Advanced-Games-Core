#include "Includes\Core\ByteBuffer.h"
#include <memory>


void ByteBuffer::Push(const uint8* b, uint32 count)
{
	m_data.resize(m_data.size() + count);
	std::memcpy(m_data.data(), b, count);
}

void ByteBuffer::Flip() 
{
	std::reverse(m_data.begin(), m_data.end());
}