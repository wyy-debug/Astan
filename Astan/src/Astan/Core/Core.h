#pragma once
#include <memory>

#include "Astan/Core/PlatformDetection.h"


#ifdef AS_DEBUG
#if defined(AS_PLATFORM_WINDOWS)
#define AS_DEBUGBREAK() __debugbreak()
#elif defined(AS_PLATFORM_LINUX)
#include <signal.h>
#define AS_DEBUGBREAK() raise(SIGTRAP)
#else
#error "Platform doesn't support debugbreak yet!"
#endif
#define AS_ENABLE_ASSERTS
#else
#define AS_DEBUGBREAK()
#endif

#define AS_EXPAND_MACRO(x) x
#define AS_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define AS_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace Astan {

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}

#include "Astan/Core/Log.h"
#include "Astan/Core/Assert.h"