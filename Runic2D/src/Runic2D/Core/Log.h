#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h" // for spdlog's custom formatting support

namespace Runic2D
{
	class RUNIC2D_API Log
	{
	public:

		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

}

// Core log macros
#define R2D_CORE_TRACE(...)    ::Runic2D::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define R2D_CORE_INFO(...)     ::Runic2D::Log::GetCoreLogger()->info(__VA_ARGS__)
#define R2D_CORE_WARN(...)     ::Runic2D::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define R2D_CORE_ERROR(...)    ::Runic2D::Log::GetCoreLogger()->error(__VA_ARGS__)
#define R2D_CORE_FATAL(...)    ::Runic2D::Log::GetCoreLogger()->fatal(__VA_ARGS__)

// Client log macros
#define R2D_TRACE(...)         ::Runic2D::Log::GetClientLogger()->trace(__VA_ARGS__)
#define R2D_INFO(...)          ::Runic2D::Log::GetClientLogger()->info(__VA_ARGS__)
#define R2D_WARN(...)          ::Runic2D::Log::GetClientLogger()->warn(__VA_ARGS__)
#define R2D_ERROR(...)         ::Runic2D::Log::GetClientLogger()->error(__VA_ARGS__)
#define R2D_FATAL(...)         ::Runic2D::Log::GetClientLogger()->fatal(__VA_ARGS__)