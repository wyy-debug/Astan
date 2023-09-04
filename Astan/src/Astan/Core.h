#pragma once

#ifdef AS_PLATFORM_WINDOWS
	#ifdef AS_BUILD_DLL
		#define ASTAN_API __declspec(dllexport)
	#else
		#define ASTAN_API __declspec(dllimport)
	#endif
	#else
	#error Astan only support Windows!
#endif

#define BIT(x) (1 << x)