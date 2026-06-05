#pragma once
#include "System.h"

#include "Runic2D/Scene/Entity.h"

namespace Runic2D {
	class RUNIC_API PhysicsSystem : public System
	{
	public:
		virtual ~PhysicsSystem() override {};
		virtual void OnStart(Scene* scene) override;
		virtual void OnStop(Scene* scene) override;
		virtual void OnUpdate(Timestep ts, Scene* scene) override;
		virtual void OnFixedUpdate(Timestep ts, Scene* scene) override;

		void InstantiatePhysics(Entity entity, Scene* scene);

	private:
		void OnRigidbodyDestroyed(entt::registry& registry, entt::entity entity);
	};
}

