#pragma once
#include <memory>

#ifdef AS_PLATFORM_WINDOWS
	#if AS_DYNAMIC_LINK
		#ifdef AS_BUILD_DLL
			#define ASTAN_API __declspec(dllexport)
		#else
			#define ASTAN_API __declspec(dllimport)
		#endif
	#else
		#define ASTAN_API
	#endif
#else
	#error Astan only support Windows!
#endif

#ifdef AS_DEBUG
	#define AS_ENABLE_ASSERTS
#endif // !AS_DEBUGAS_ENABLE_ASSERTS
//
//

#ifdef AS_ENABLE_ASSERTS
	#define AS_ASSERT(x, ...) {if(!(x)) { AS_ERROR("Assertion Failed: {0}", __VA_ARGS__);__debugbreak(); }}
	#define AS_CORE_ASSERT(x, ...) {if(!(x)) { AS_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__);__debugbreak(); }}
#else
	#define AS_ASSERT(x, ...)
	#define AS_CORE_ASSERT(x, ...)
#endif  


#define BIT(x) (1 << x)


#define AS_BIND_EVENT_FN(fn) [this](auto&... args) -> decltype(auto) {return this->fn(std::forward<decltype(args)>(args)...);}

namespace Astan{
	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T,typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
	
	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T,typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}