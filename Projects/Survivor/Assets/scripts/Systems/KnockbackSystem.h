#pragma once
#include "Runic2D/Systems/System.h"

using namespace Runic2D;

namespace Survivor {
	class KnockbackSystem : public Runic2D::System {
	public:
		virtual ~KnockbackSystem() override {};
		virtual void OnFixedUpdate(Runic2D::Timestep ts, Runic2D::Scene* scene) override;
	};
}

