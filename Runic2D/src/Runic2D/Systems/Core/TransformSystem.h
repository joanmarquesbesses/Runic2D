#pragma once
#include "Runic2D/Systems/System.h"

namespace Runic2D {
	class RUNIC_API TransformSystem : public System
	{
	public:
		virtual ~TransformSystem() override {};
		virtual void OnUpdate(Timestep ts, Scene* scene) override;
	};
}


