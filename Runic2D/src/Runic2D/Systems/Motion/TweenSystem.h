#pragma once
#include "Runic2D/Systems/System.h"

#include "glm/glm.hpp"

namespace Runic2D {
	class RUNIC_API TweenSystem : public System
	{
	public:
		virtual ~TweenSystem() override {};
		virtual void OnUpdate(Timestep ts, Scene* scene) override;
	};
}


