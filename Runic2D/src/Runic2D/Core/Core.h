#pragma once

#include <memory>

#ifdef RUNIC2D_PLATFORM_WINDOWS
#if RUNIC2D_DYNAMIC_LINK
	#ifdef RUNIC2D_BUILD_DLL
		#define RUNIC2D_API __declspec(dllexport)
	#else
		#define RUNIC2D_API __declspec(dllimport)
	#endif // DEBUG
#else
	#define RUNIC2D_API
#endif
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