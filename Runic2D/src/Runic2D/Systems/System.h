#pragma once

#include "Runic2D/Core/Core.h"
#include "Runic2D/Core/Timestep.h"

namespace Runic2D {
	class Scene;

	class RUNIC_API System {
	public:
		virtual ~System() = default;
		virtual void OnStart(Scene* scene) {};
		virtual void OnStop(Scene* scene) {};
		virtual void OnUpdate(Timestep ts, Scene* scene) {};
		virtual void OnFixedUpdate(Timestep ts, Scene* scene) {};
	};
}
