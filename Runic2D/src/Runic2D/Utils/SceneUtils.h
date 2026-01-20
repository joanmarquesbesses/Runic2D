#pragma once

#include <glm/glm.hpp>

namespace Runic2D {
	class Scene;
}

namespace Runic2D::Utils {

	class SceneUtils
	{
	public:
		static glm::vec2 GetMouseWorldPosition(Scene* scene);
	};

}