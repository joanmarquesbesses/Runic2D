#pragma once

#include "Runic2D/Systems/System.h"

namespace Runic2D {

	class RUNIC_API FlockingSystem : public System
	{
	public:
		virtual void OnUpdate(Timestep ts, Scene* scene) override;
	};

}