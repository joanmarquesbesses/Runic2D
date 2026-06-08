#pragma once
#include "System.h"

#include "glm/glm.hpp"

namespace Runic2D {
	class RUNIC_API UISystem : public System
	{
	public:
		virtual ~UISystem() override {};
		virtual void OnUpdate(Timestep ts, Scene* scene) override;
		virtual void OnRender(Scene* scene) override;

		glm::vec2 GetMousePositionInUISpace(Scene* scene);
	};
}

