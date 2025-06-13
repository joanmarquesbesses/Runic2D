#pragma once

#ifdef RUNIC2D_PLATFORM_WINDOWS
	#ifdef RUNIC2D_BUILD_DLL
		#define RUNIC2D_API __declspec(dllexport)
    #else
		#define RUNIC2D_API __declspec(dllimport)
	#endif // DEBUG
#else
	#error Runic2D only supports Windows!
#endif // HZ_PLATFORM_WINDOWS

#define BIT(x) (1 << x)
