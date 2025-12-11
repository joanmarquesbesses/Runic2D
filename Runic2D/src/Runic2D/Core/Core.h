#pragma once

#include <memory>

#include "Runic2D/Core/PlatformDetection.h"

#ifdef R2D_DEBUG
	#if defined(R2D_PLATFORM_WINDOWS)
		#define R2D_DEBUGBREAK() __debugbreak()
	#elif defined(R2D_PLATFORM_LINUX)
		#include <signal.h>
		#define R2D_DEBUGBREAK() raise(SIGTRAP)
	#else
		#error "Platform doesn't support debugbreak yet!"
	#endif
	#define R2D_ENABLE_ASSERTS
#else
	#define R2D_DEBUGBREAK()
#endif

#define R2D_EXPAND_MACRO(x) x
#define R2D_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define R2D_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace Runic2D{

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename... Args>
	constexpr Scope<T> CreateScope(Args&&... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename... Args>
	constexpr Ref<T> CreateRef(Args&&... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

} // namespace Runic2D

#include "Runic2D/Core/Log.h"
#include "Runic2D/Core/Assert.h"