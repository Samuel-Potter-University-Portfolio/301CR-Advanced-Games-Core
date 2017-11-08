#pragma once
#include "Common.h"
#include <vector>


/**
* Represents a structure for IO using binary data
*/
class CORE_API ByteBuffer 
{
private:
	std::vector<uint8> m_data;

public:

	/**
	* Flips the buffer making it ready to be read/written to
	*/
	void Flip();

	/**
	* Getters & Setters
	*/
public:
	inline const uint8* Data() const { return m_data.data(); }

	inline void Clear() { m_data.clear(); }
	inline void Reserve(const uint32 size) { m_data.reserve(size); }
	inline const uint32 Size() const { return m_data.size(); }

	void Push(const uint8* b, uint32 count);
	inline void Push(const uint8& b) { m_data.emplace_back(b); }

	inline uint8 Pop() { const uint8 b = m_data.back(); m_data.pop_back(); return b; }
	inline const uint8& Peek() const { return m_data.back(); }
};