#pragma once
#include "Runic2D/Systems/System.h"

using namespace Runic2D;

namespace Survivor {
	class EnemySystem : public Runic2D::System
	{
	public:
		virtual ~EnemySystem() override {};
		virtual void OnUpdate(Timestep ts, Scene* scene) override;
	};
}

