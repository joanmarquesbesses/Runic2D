#pragma once
#include "System.h"

#include "Runic2D/Scene/Entity.h"

namespace Runic2D {
	class RUNIC_API ScriptingSystem : public System
	{
	public:
		virtual ~ScriptingSystem() override {};
		virtual void OnStart(Scene* scene) override;
		virtual void OnStop(Scene* scene) override;
		virtual void OnUpdate(Timestep ts, Scene* scene) override;
		virtual void OnFixedUpdate(Timestep ts, Scene* scene) override;

	private:
		void OnNativeScriptDestroyed(entt::registry& registry, entt::entity entity);
	};
}

