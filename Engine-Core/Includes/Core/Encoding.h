/**
* Encoding and Decoding template overrides for commonly used types (Further implementations should override these for themselves!)
*/
#pragma once
#include "Common.h"

#include <stack>
#include <memory>


typedef std::stack<uint8> ByteBuffer;



template<typename T>
void Encode(ByteBuffer& buffer, T data)
{
	static_assert(false, "No encode implementation");
}


template<>
void Encode<int>(ByteBuffer& buffer, int32 data)
{
	buffer.push(data >> 0 * 8);
	buffer.push(data >> 1 * 8);
	buffer.push(data >> 2 * 8);
	buffer.push(data >> 3 * 8);
}

template<>
void Encode<float>(ByteBuffer& buffer, float data)
{
	int temp;
	std::memcpy(&temp, &data, sizeof(float));
	Encode(buffer, temp);
}

template<>
void Encode<std::string>(ByteBuffer& buffer, string data)
{
	buffer.push('\0');
	for (int i = data.length() - 1; i >= 0; --i)
		buffer.push(data[i]);
}

template<>
void Encode<const char*>(ByteBuffer& buffer, const char* data)
{
	std::string str = data;
	Encode(buffer, str);
}


template<typename T>
void Decode(ByteBuffer& buffer, T& out)
{
	static_assert(false, "No decode implementation");
}

template<>
void Decode<int>(ByteBuffer& buffer, int32& out)
{
	out = 0;

	out += (int)(buffer.top()) << 3 * 8;
	buffer.pop();

	out += (int)(buffer.top()) << 2 * 8;
	buffer.pop();

	out += (int)(buffer.top()) << 1 * 8;
	buffer.pop();

	out += (int)(buffer.top()) << 0 * 8;
	buffer.pop();
}

template<>
void Decode<float>(ByteBuffer& buffer, float& out)
{
	int temp;
	Decode(buffer, temp);
	std::memcpy(&out, &temp, sizeof(float));
}

template<>
void Decode<std::string>(ByteBuffer& buffer, string& out)
{
	while (true)
	{
		char c = buffer.top();
		buffer.pop();

		if (c == '\0')
			return;

		out += c;
	}
}