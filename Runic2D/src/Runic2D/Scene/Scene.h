#pragma once

#include <entt.hpp>

#include "Runic2D/Core/Core.h"
#include "Runic2D/Core/Timestep.h"

namespace Runic2D {

	class Entity;

	class Scene {
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		void OnUpdate(Timestep ts);
		void OnViewportResize(uint32_t width, uint32_t height);
	private:
		void OnCameraComponentConstruct(entt::registry& registry, entt::entity entity);
	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		friend class Entity;
		friend class SceneHierarchyPanel;
	};
}