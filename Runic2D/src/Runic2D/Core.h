#pragma once

#ifdef RUNIC2D_PLATFORM_WINDOWS
	#ifdef RUNIC2D_BUILD_DLL
		#define RUNIC2D_API __declspec(dllexport)
    #else
		#define RUNIC2D_API __declspec(dllimport)
	#endif // DEBUG
#else
	#error Runic2D only supports Windows!
#endif // RUNIC2D_PLATFORM_WINDOWS

#ifdef RUNIC2D_DEBUG
	#define R2D_ENABLE_ASSERTS
#endif // RUNIC2D_DEBUG


#ifdef R2D_ENABLE_ASSERTS
	#define R2D_ASSERT(x, ...) { if(!(x)) { R2D_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define R2D_CORE_ASSERT(x, ...) { if(!(x)) { R2D_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define R2D_ASSERT(x, ...)
	#define R2D_CORE_ASSERT(x, ...)
#endif // RUNIC2D_ENABLE_ASSERTS

#define BIT(x) (1 << x)

#define RUNIC2D_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)