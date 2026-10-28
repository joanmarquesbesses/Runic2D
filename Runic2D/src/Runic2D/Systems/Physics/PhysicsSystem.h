#pragma once
#include "Runic2D/Systems/System.h"

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
		void SetCollisionEnabled(Entity entity, bool enabled);
		void UpdateEntityColliders(Entity entity);

		static void SetLayerNames(const std::vector<std::string>& names) { s_LayerNames = names; }
		static const std::vector<std::string>& GetLayerNames() { return s_LayerNames; }

	private:
		void OnRigidbodyDestroyed(entt::registry& registry, entt::entity entity);

		inline static std::vector<std::string> s_LayerNames;
	};
}


