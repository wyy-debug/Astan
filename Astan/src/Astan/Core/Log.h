#pragma once

#include "Core.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"


#pragma warning(push,0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace Astan {

	class Log
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

template<typename OStream, glm::length_t L,typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::vec<L, T, Q>& vector)
{
	return os << glm::to_string(vector);
}

template<typename OStream, glm::length_t C, glm::length_t R,typename T,glm::qualifier Q>
inline OStream& operator<<(OStream& os, const glm::mat<C, R, T, Q>& matrix)
{
	return os << glm::to_string(matrix);
}

template<typename OStream, typename T, glm::qualifier Q>
inline OStream& operator<<(OStream& os, glm::qua<T, Q> quaternio)
{
	return os << glm::to_string(quaternio);
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
