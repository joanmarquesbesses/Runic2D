#pragma once

#include "Runic2D/Core/Core.h"
#include "Runic2D/Core/Log.h"
#include <filesystem>

#ifdef R2D_ENABLE_ASSERTS

// Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
// provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
#define R2D_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { R2D##type##ERROR(msg, __VA_ARGS__); R2D_DEBUGBREAK(); } }
#define R2D_INTERNAL_ASSERT_WITH_MSG(type, check, ...) R2D_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define R2D_INTERNAL_ASSERT_NO_MSG(type, check) R2D_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", R2D_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)
		
#define R2D_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define R2D_INTERNAL_ASSERT_GET_MACRO(...) R2D_EXPAND_MACRO( R2D_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, R2D_INTERNAL_ASSERT_WITH_MSG, R2D_INTERNAL_ASSERT_NO_MSG) )

// Currently accepts at least the condition and one additional parameter (the message) being optional
#define R2D_ASSERT(...) R2D_EXPAND_MACRO( R2D_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
#define R2D_CORE_ASSERT(...) R2D_EXPAND_MACRO( R2D_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
#define R2D_ASSERT(...)
#define R2D_CORE_ASSERT(...)
#endif
