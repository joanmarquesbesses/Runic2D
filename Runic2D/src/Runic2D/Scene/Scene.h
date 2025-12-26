#pragma once

#include "Runic2D/Core/Timestep.h"
#include "Runic2D/Core/UUID.h"
#include "Runic2D/Renderer/EditorCamera.h"

#include <entt.hpp>
#include <box2d/types.h>

namespace Runic2D {

	class Entity;

	struct TransformComponent;

	class Scene {
	public:
		Scene();
		~Scene();

		static Ref<Scene> Copy(Ref<Scene> other);

		Entity CreateEntity(const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		Entity GetEntityByUUID(UUID uuid);

		void OnUpdateRunTime(Timestep ts);
		void OnUpdateEditor(Timestep ts, EditorCamera& camera);
		void OnViewportResize(uint32_t width, uint32_t height);

		void OnRuntimeStart();
		void OnRuntimeStop();

		Entity GetPrimaryCameraEntity();

		void SetCollisionEnabled(Entity entity, bool enabled);

		void ParentEntity(Entity entity, Entity parent);
		void UnparentEntity(Entity entity, bool convertToWorldSpace = true);

		glm::mat4 GetWorldTransform(Entity entity);
		glm::mat4 GetWorldTransform(const TransformComponent& transform, Entity entity);

		void DuplicateEntity(Entity entity);

		void OnRenderOverlay(const glm::mat4& viewProjection);

		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	private:
		void OnCameraComponentConstruct(entt::registry& registry, entt::entity entity);
		void CopyEntity(Entity src, Entity dst);
	private:
		entt::registry m_Registry;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

		b2WorldId m_PhysicsWorld = b2_nullWorldId;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}