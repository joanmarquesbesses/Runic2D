#pragma once

#include "Runic2D/Core/Timestep.h"
#include "Runic2D/Core/UUID.h"
#include "Runic2D/Renderer/EditorCamera.h"

#include <entt.hpp>

namespace Runic2D {

	class Entity;

	class Scene {
	public:
		Scene();
		~Scene();

		Entity CreateEntity(const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());

		void OnUpdateRunTime(Timestep ts);
		void OnUpdateEditor(Timestep ts, EditorCamera& camera);
		void OnViewportResize(uint32_t width, uint32_t height);

		Entity GetPrimaryCameraEntity();
	private:
		void OnCameraComponentConstruct(entt::registry& registry, entt::entity entity);
	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}