#include "Includes\Core\ByteBuffer.h"
#include <memory>


ByteBuffer::ByteBuffer() 
{

}

void ByteBuffer::Push(uint8* b, uint32 count) 
{
	m_data.reserve(m_data.size() + count);
	std::memcpy(&m_data.back(), b, count);
}