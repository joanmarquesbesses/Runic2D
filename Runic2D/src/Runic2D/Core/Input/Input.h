#pragma once

#include "Runic2D/Core/Base/Core.h"
#include "Runic2D/Core/Input/KeyCodes.h"
#include "Runic2D/Core/Input/MouseButtonCodes.h"

#include <glm/glm.hpp>

namespace Runic2D
{
	class RUNIC_API Input
	{
	public:
		static bool IsKeyPressed(KeyCode keycode); // Use KeyCode, not int directly

		static bool IsMouseButtonPressed(MouseButton button); // Use MouseButtonCode, not int directly
		static glm::vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};
}

