#pragma once

#include "Entity.h"

namespace Runic2D
{
	class ScriptableEntity {
	public:
		virtual ~ScriptableEntity() = default;
		template<typename T>
		T& GetComponent() {
			return m_Entity.GetComponent<T>();
		}

		template<typename T>
		bool HasComponent() {
			return m_Entity.HasComponent<T>();
		}

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args) {
			return m_Entity.AddComponent<T>(std::forward<Args>(args)...);
		}

		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args) {
			return m_Entity.AddOrReplaceComponent<T>(std::forward<Args>(args)...);
		}

		template<typename T, typename... Args>
		T& GetOrAddComponent(Args&&... args) {
			return m_Entity.GetOrAddComponent<T>(std::forward<Args>(args)...);
		}

		template<typename T>
		void RemoveComponent() {
			m_Entity.RemoveComponent<T>();
		}

		Entity GetEntity() const { return m_Entity; }
		Scene* GetScene() const { return m_Entity.GetScene(); }
	protected:
		virtual void OnCreate() {}
		virtual void OnDestroy() {}
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnFixedUpdate(Timestep ts) {}
		virtual void OnSensor(Entity other) {}

		virtual void OnCollision(Entity other) {}

		virtual bool UpdateWhenPaused() { return false; }

		void Destroy()
		{
			m_Entity.Destroy();
		}
	private:
		Entity m_Entity;
		friend class ScriptingSystem;
		friend class PhysicsSystem;
	};
}
