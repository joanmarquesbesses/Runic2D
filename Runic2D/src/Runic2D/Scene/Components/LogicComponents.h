#pragma once

#include "Runic2D/Core/Base/Core.h"
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Runic2D {
	
	struct RUNIC_API LifetimeComponent
	{
		float TimeRemaining = 1.0f;
	};
}
