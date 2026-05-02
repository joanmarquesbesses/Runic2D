#include "R2Dpch.h"
#include "InputManager.h"
#include "Runic2D/Core/Input.h"
#include "Runic2D/Core/Application.h"

#ifndef R2D_DIST
#include <imgui.h> 
#endif

namespace Runic2D {

	std::unordered_map<int, std::unordered_map<std::string, ActionBinding>> InputManager::s_Bindings;

	void InputManager::BindAction(const std::string& actionName, KeyCode key, int playerIndex)
	{
		s_Bindings[playerIndex][actionName] = { InputDevice::Keyboard, static_cast<int>(key) };
	}

	void InputManager::BindAction(const std::string& actionName, MouseButton button, int playerIndex)
	{
		s_Bindings[playerIndex][actionName] = { InputDevice::Mouse, static_cast<int>(button) };
	}

	bool InputManager::IsActionPressed(const std::string& actionName, int playerIndex)
	{
#ifndef R2D_DIST
		if (!s_ViewportFocused)
			return false;
#endif

		auto playerIt = s_Bindings.find(playerIndex);
		if (playerIt == s_Bindings.end()) return false; 

		auto actionIt = playerIt->second.find(actionName);
		if (actionIt == playerIt->second.end()) return false; 

		const ActionBinding& binding = actionIt->second;

		if (binding.Device == InputDevice::Keyboard)
		{
			return Input::IsKeyPressed(static_cast<KeyCode>(binding.Code));
		}
		else if (binding.Device == InputDevice::Mouse)
		{
			return Input::IsMouseButtonPressed(static_cast<MouseButton>(binding.Code));
		}

		return false;
	}
}