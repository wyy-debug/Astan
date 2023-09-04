#pragma once
#include <memory>
#include "Core.h"
#include "spdlog/spdlog.h"

namespace Astan {

	class ASTAN_API Log
	{
	public:
		static void Init();
		inline static std::shared_ptr<spdlog::logger> GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger> GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}
//core log
#define AS_CORE_TRACE(...)    ::Astan::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define AS_CORE_INFO(...)     ::Astan::Log::GetCoreLogger()->info(__VA_ARGS__)
#define AS_CORE_WARN(...)     ::Astan::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define AS_CORE_ERROR(...)    ::Astan::Log::GetCoreLogger()->error(__VA_ARGS__)
#define AS_CORE_FATAL(...)    ::Astan::Log::GetCoreLogger()-fatal(__VA_ARGS__)

//client log
#define AS_TRACE(...)    ::Astan::Log::GetClientLogger()->trace(__VA_ARGS__)
#define AS_INFO(...)     ::Astan::Log::GetClientLogger()->info(__VA_ARGS__)
#define AS_WARN(...)     ::Astan::Log::GetClientLogger()->warn(__VA_ARGS__)
#define AS_ERROR(...)    ::Astan::Log::GetClientLogger()->error(__VA_ARGS__)
#define AS_FATAL(...)    ::Astan::Log::GetClientLogger()-fatal(__VA_ARGS__)
