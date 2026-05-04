#pragma once

#include "Runic2D/Core/Timestep.h"
#include "Runic2D/Core/UUID.h"
#include "Runic2D/Renderer/EditorCamera.h"
#include "Runic2D/Renderer/ParticleSystem.h"

#include <entt.hpp>
#include <box2d/types.h>

namespace Runic2D {

	class Entity;

	struct TransformComponent;

	struct SceneStats
	{
		uint32_t TotalEntities = 0;
		uint32_t ScriptUpdates = 0;
		uint32_t ActiveParticles = 0;
	};

	class RUNIC_API Scene {
	public:
		Scene();
		~Scene();

		static Ref<Scene> Copy(Ref<Scene> other);

		Entity CreateEntity(const std::string& name = std::string());
		void DestroyEntity(Entity entity);
		void SubmitForDestruction(Entity entity);

		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		Entity GetEntityByUUID(UUID uuid);

		// Runtime
		void OnUpdateRunTime(Timestep ts);
		void OnUpdateEditor(Timestep ts, EditorCamera& camera);
		void UpdateUIInteraction();

		void SetPaused(bool paused) { m_IsPaused = paused; }
		bool IsPaused() const { return m_IsPaused; }

		// Rendering
		void OnRenderRuntime();
		void OnRenderUI();

		void OnViewportResize(uint32_t width, uint32_t height);

		uint32_t GetViewportWidth() const { return m_ViewportWidth; }
		uint32_t GetViewportHeight() const { return m_ViewportHeight; }

		glm::vec2 GetMousePositionInUISpace();

		void SetViewportBounds(const glm::vec2& boundsMin, const glm::vec2& boundsMax) 
		{
			m_ViewportBoundsMin = boundsMin;
			m_ViewportBoundsMax = boundsMax;
		}

		void OnRuntimeStart();
		void OnRuntimeStop();

		void InstantiatePhysics(Entity entity);
		void UpdateEntityColliders(Entity entity);

		void EmitParticles(const ParticleProps& props) { m_ParticleSystem.Emit(props); }

		Entity GetPrimaryCameraEntity();

		void SetCollisionEnabled(Entity entity, bool enabled);

		void ParentEntity(Entity entity, Entity parent);
		void UnparentEntity(Entity entity, bool convertToWorldSpace = true);

		Entity FindEntityByName(std::string_view name);

		glm::mat4 GetWorldTransform(Entity entity);
		glm::mat4 GetWorldTransform(const TransformComponent& transform, Entity entity);

		void DuplicateEntity(Entity entity);

		void OnRenderOverlay(const glm::mat4& viewProjection);

		void UpdateAnimation(Timestep ts);

		int GetActiveParticleCount() const { return m_ParticleSystem.GetActiveParticleCount(); }

		SceneStats GetStats() const;

		template<typename T>
		void OnComponentAdded(Entity entity, T& component) {}

		int GetSizeOfAllEntities() const { 
			int count = 0;

			m_Registry.view<entt::entity>().each([&](auto entityID) {
					count++;
				}
			);

			return count;
		}

		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return m_Registry.view<Components...>();
		}

		template<typename T>
		Entity GetEntityWithComponent()
		{
			auto view = m_Registry.view<T>();
			if (view.begin() != view.end())
			{
				return { *view.begin(), this };
			}
			return {}; 
		}

	private:
		void OnCameraComponentConstruct(entt::registry& registry, entt::entity entity);
		void CopyEntity(Entity src, Entity dst);

		void UpdateScripts(Timestep ts);
		void UpdatePhysics(Timestep ts);

	private:
		entt::registry m_Registry;
		bool m_IsPaused = false;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		glm::vec2 m_ViewportBoundsMin = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBoundsMax = { 0.0f, 0.0f };

		b2WorldId m_PhysicsWorld = b2_nullWorldId;

		ParticleSystem m_ParticleSystem;

		std::vector<entt::entity> m_DestructionQueue;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}