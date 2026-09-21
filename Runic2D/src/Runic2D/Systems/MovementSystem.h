#pragma once
#include "Runic2D/Systems/System.h"

namespace Runic2D {
	class MovementSystem : public Runic2D::System
	{
	public:
		virtual ~MovementSystem() override {};
		virtual void OnFixedUpdate(Timestep ts, Scene* scene) override;
	};
}


