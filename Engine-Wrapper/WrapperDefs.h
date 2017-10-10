#pragma once
/**
* Definitions used by wrapper
*/
#pragma once

#ifdef BUILD_WRAPPER
#define WRAPPER_API __declspec(dllexport)
#else
#define WRAPPER_API __declspec(dllimport)
#endif