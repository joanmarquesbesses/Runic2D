#pragma once

#include "Runic2D/Systems/System.h"

namespace Survivor {
	class DamageFlashSystem : public Runic2D::System {
	public:
		virtual ~DamageFlashSystem() override {};
		virtual void OnUpdate(Runic2D::Timestep ts, Runic2D::Scene* scene) override;
	};
}

