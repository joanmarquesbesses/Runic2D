#pragma once

#include "Runic2D/Core/Core.h"
#include "Runic2D/Core/KeyCodes.h"
#include "Runic2D/Core/MouseButtonCodes.h"

#include <glm/glm.hpp>

namespace Runic2D
{
	class RUNIC2D_API Input
	{
	public:
		static bool IsKeyPressed(KeyCode keycode); // Use KeyCode, not int directly

		static bool IsMouseButtonPressed(MouseButton button); // Use MouseButtonCode, not int directly
		static glm::vec2 GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};
}