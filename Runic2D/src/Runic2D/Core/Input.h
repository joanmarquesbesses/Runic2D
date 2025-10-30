#pragma once

#include "Runic2D/Core/Core.h"
//#include "Runic2D/Core/KeyCodes.h"
//#include "Runic2D/Core/MouseButtonCodes.h"

namespace Runic2D
{
	class RUNIC2D_API Input
	{
	public:
		static bool IsKeyPressed(int keycode); // Use KeyCode, not int directly

		static bool IsMouseButtonPressed(int button); // Use MouseButtonCode, not int directly
		static std::pair<float, float> GetMousePosition();
		static float GetMouseX();
		static float GetMouseY();
	};
}