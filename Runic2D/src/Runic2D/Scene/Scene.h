#pragma once

#include "Runic2D/Core/Base/Timestep.h"
#include "Runic2D/Core/Base/UUID.h"
#include "Runic2D/Renderer/EditorCamera.h"

#include <entt.hpp>
#include <box2d/types.h>
#include <unordered_map>
#include <typeindex>

namespace Runic2D {

	class System;
	class Entity;

	struct TransformComponent;

	enum class SystemPhase {
		Logic,			// Primer (Scripts, Moviment Enemics, IA...)
		Physics,		// Segon (Box2D)
		PostUpdate,		// Tercer (Tweens, TransformSystem, Particles...)
		Render
	};

	class RUNIC_API Scene {
	public:
		Scene();
		~Scene();

		static Ref<Scene> Copy(Ref<Scene> other);

		Entity CreateEntity(const std::string& name = std::string());
		void DestroyEntity(Entity entity);
		void SubmitForDestruction(Entity entity);
		void DuplicateEntity(Entity entity);
		
		Entity FindEntityByName(std::string_view name);

		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		Entity GetEntityByUUID(UUID uuid);

		// Runtime
		void OnUpdateRunTime(Timestep ts);
		void OnFixedUpdateRunTime(Timestep ts);
		void OnUpdateEditor(Timestep ts, EditorCamera& camera);

		void SetPaused(bool paused) { m_IsPaused = paused; }
		bool IsPaused() const { return m_IsPaused; }

		// Rendering
		void OnRenderRuntime();

		void OnViewportResize(uint32_t width, uint32_t height);

		uint32_t GetViewportWidth() const { return m_ViewportWidth; }
		uint32_t GetViewportHeight() const { return m_ViewportHeight; }
		glm::vec2 GetViewportBoundsMin() const { return m_ViewportBoundsMin; }
		glm::vec2 GetViewportBoundsMax() const { return m_ViewportBoundsMax; }

		void SetViewportBounds(const glm::vec2& boundsMin, const glm::vec2& boundsMax) 
		{
			m_ViewportBoundsMin = boundsMin;
			m_ViewportBoundsMax = boundsMax;
		}

		void OnRuntimeStart();
		void OnRuntimeStop();

		Entity GetPrimaryCameraEntity();

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

		b2WorldId& GetPhysicsWorldID() {
			return m_PhysicsWorld;
		}

		entt::registry& GetEntityRegistry() {
			return m_Registry;
		}

		const entt::registry& GetEntityRegistry() const {
			return m_Registry;
		}

	public:
		template<typename T>
		void AddSystem(Ref<T> system, std::initializer_list<SystemPhase> phases = { SystemPhase::Logic }) {
			m_SystemsMap[typeid(T)] = system;

			for (auto phase : phases)
			{
				switch (phase) {
					case SystemPhase::Logic:      m_LogicSystems.push_back(system); break;
					case SystemPhase::Physics:    m_PhysicsSystems.push_back(system); break;
					case SystemPhase::PostUpdate: m_PostUpdateSystems.push_back(system); break;
					case SystemPhase::Render:     m_RenderSystems.push_back(system); break;
				}
			}
		}

		template<typename T>
		Ref<T> GetSystem() const {
			auto it = m_SystemsMap.find(typeid(T));
			if (it != m_SystemsMap.end()) {
				return std::static_pointer_cast<T>(it->second);
			}
			return nullptr;
		}

	private:
		void OnCameraComponentConstruct(entt::registry& registry, entt::entity entity);
		void CopyEntity(Entity src, Entity dst);

	private:
		entt::registry m_Registry;
		bool m_IsPaused = false;
		bool m_ShowDebugOverlay = false;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		glm::vec2 m_ViewportBoundsMin = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBoundsMax = { 0.0f, 0.0f };

		b2WorldId m_PhysicsWorld = b2_nullWorldId;

		std::unordered_map<std::type_index, Ref<System>> m_SystemsMap;
		std::vector<Ref<System>> m_LogicSystems;
		std::vector<Ref<System>> m_PhysicsSystems;
		std::vector<Ref<System>> m_PostUpdateSystems;
		std::vector<Ref<System>> m_RenderSystems;

		std::vector<entt::entity> m_DestructionQueue;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}
