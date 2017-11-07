#pragma once
#include "Common.h"
#include "Encoding.h"


/**
* Holds version numbering information
*/
struct CORE_API Version 
{
	uint16 major;
	uint16 minor;
	uint16 patch;

	Version(uint32 major = 0, uint32 minor = 0, uint32 patch = 0) : major(major), minor(minor), patch(patch)
	{}


	inline bool operator==(const Version& other) const
	{
		return major == other.major && minor == other.minor && patch == other.patch;
	}
	inline bool operator!=(const Version& other) const
	{
		return major != other.major || minor != other.minor || patch != other.patch;
	}

	inline bool operator<(const Version& other) const
	{
		return major < other.major || (major == other.major && other.minor < other.minor) || (major == other.major && minor == other.minor && patch < other.patch);
	}
	inline bool operator<=(const Version& other) const
	{
		return major <= other.major || (major == other.major && other.minor <= other.minor) || (major == other.major && minor == other.minor && patch <= other.patch);
	}

	inline bool operator>(const Version& other) const
	{
		return major > other.major || (major == other.major && other.minor > other.minor) || (major == other.major && minor == other.minor && patch > other.patch);
	}
	inline bool operator>=(const Version& other) const
	{
		return major >= other.major || (major == other.major && other.minor >= other.minor) || (major == other.major && minor == other.minor && patch >= other.patch);
	}
};


template<>
inline void Encode<Version>(ByteBuffer& buffer, const Version& data)
{
	Encode<uint16>(buffer, data.major);
	Encode<uint16>(buffer, data.minor);
	Encode<uint16>(buffer, data.patch);
}

template<>
inline bool Decode<Version>(ByteBuffer& buffer, Version& out)
{
	return	Decode<uint16>(buffer, out.major) &&
			Decode<uint16>(buffer, out.minor) &&
			Decode<uint16>(buffer, out.patch);
}