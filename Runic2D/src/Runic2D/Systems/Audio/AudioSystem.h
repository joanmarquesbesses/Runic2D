#pragma once

#include "Runic2D/Systems/System.h"

namespace Runic2D {

	class RUNIC_API AudioSystem : public System
	{
	public:
		AudioSystem() = default;
		virtual ~AudioSystem() = default;

		virtual void OnStart(Scene* scene) override;
		virtual void OnStop(Scene* scene) override;
		virtual void OnUpdate(Timestep ts, Scene* scene) override;
	};

}
