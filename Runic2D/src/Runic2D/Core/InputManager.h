#pragma once

#include "Runic2D/Core/Core.h"
#include "Runic2D/Core/KeyCodes.h"
#include "Runic2D/Core/MouseButtonCodes.h"
#include <string>
#include <unordered_map>

namespace Runic2D {

	enum class InputDevice { Keyboard, Mouse, Gamepad };

	struct ActionBinding {
		InputDevice Device;
		int Code;
	};

	class RUNIC_API InputManager
	{
	public:
		static void BindAction(const std::string& actionName, KeyCode key, int playerIndex = 0);
		static void BindAction(const std::string& actionName, MouseButton button, int playerIndex = 0);

		static bool IsActionPressed(const std::string& actionName, int playerIndex = 0);

#ifndef R2D_DIST
		static void SetViewportFocused(bool focused) { s_ViewportFocused = focused; }
		static bool IsViewportFocused() { return s_ViewportFocused; }
#endif

	private:
		static std::unordered_map<int, std::unordered_map<std::string, ActionBinding>> s_Bindings;

#ifndef R2D_DIST
		inline static bool s_ViewportFocused = true;
#endif
	};
}