#pragma once

#include "Runic2D/Core/Timestep.h"
#include "Runic2D/Core/UUID.h"
#include "Runic2D/Renderer/EditorCamera.h"
#include "Runic2D/Renderer/ParticleSystem.h"

#include <entt.hpp>
#include <box2d/types.h>
#include <unordered_map>
#include <typeindex>

namespace Runic2D {

	class System;
	class Entity;

	struct TransformComponent;

	struct SceneStats
	{
		uint32_t TotalEntities = 0;
		uint32_t ScriptUpdates = 0;
		uint32_t ActiveParticles = 0;
	};

	enum class SystemPhase {
		Logic,			// Primer (Scripts, Moviment Enemics, IA...)
		Physics,		// Segon (Box2D)
		PostUpdate		// Tercer (Tweens, TransformSystem, Particles...)
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
		void OnFixedUpdateRunTime(Timestep ts);
		void OnUpdateEditor(Timestep ts, EditorCamera& camera);
		void UpdateUIInteraction();

		void SetPaused(bool paused) { m_IsPaused = paused; }

		bool IsDebugOverlayEnabled() const { return m_ShowDebugOverlay; }
		void SetDebugOverlayEnabled(bool enabled) { m_ShowDebugOverlay = enabled; }
		bool IsPaused() const { return m_IsPaused; }

		// Rendering
		void OnRenderRuntime();
		void OnRenderUI();
		void OnRenderDebugOverlay();

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

		void UpdateEntityColliders(Entity entity);

		void EmitParticles(const ParticleProps& props) { m_ParticleSystem.Emit(props); }

		Entity GetPrimaryCameraEntity();

		void SetCollisionEnabled(Entity entity, bool enabled);

		Entity FindEntityByName(std::string_view name);

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
		void AddSystem(Ref<T> system, SystemPhase phase) {
			m_SystemsMap[typeid(T)] = system;

			switch (phase) {
				case SystemPhase::Logic:      m_LogicSystems.push_back(system); break;
				case SystemPhase::Physics:    m_PhysicsSystems.push_back(system); break;
				case SystemPhase::PostUpdate: m_PostUpdateSystems.push_back(system); break;
			}
		}

		template<typename T>
		Ref<T> GetSystem() {
			auto it = m_SystemsMap.find(typeid(T));
			if (it != m_SystemsMap.end()) {
				return std::static_pointer_cast<T>(it->second);
			}
			return nullptr;
		}

	private:
		void OnCameraComponentConstruct(entt::registry& registry, entt::entity entity);
		void CopyEntity(Entity src, Entity dst);

		void UpdateTweens(Timestep ts);

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

		ParticleSystem m_ParticleSystem;

		std::vector<entt::entity> m_DestructionQueue;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};
}