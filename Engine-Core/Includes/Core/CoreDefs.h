/**
* Definitions used by core
*/
#pragma once

#ifdef BUILD_CORE
#define CORE_API __declspec(dllexport)
#else
#define CORE_API __declspec(dllimport)
#endif