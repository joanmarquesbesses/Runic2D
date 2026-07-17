#pragma once

#include "Runic2D/Core/Base/Core.h"

#include <glm/glm.hpp>

namespace Runic2D::Math {
	RUNIC_API bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
}
