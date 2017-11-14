#pragma once
/**
* All commonly required types and names required by the engine
*/

#include <string>
#include <SFML\System\Vector2.hpp>
#include <SFML\System\Vector3.hpp>

typedef std::string	string;

typedef __int8 int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;

typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;


typedef sf::Vector2<float>	vec2;
typedef sf::Vector3<float>	vec3;
typedef sf::Vector2<int32>	ivec2;
typedef sf::Vector3<int32>	ivec3;
typedef sf::Vector2<uint32>	uvec2;
typedef sf::Vector3<uint32>	uvec3;
typedef sf::Vector2<uint8>	bvec2;
typedef sf::Vector3<uint8>	bvec3;