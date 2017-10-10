#pragma once
#include <msclr\marshal_cppstd.h>
#include <string>


template<typename To, typename From>
inline To Convert(From in)
{
	return msclr::interop::marshal_as<To>(in);
}