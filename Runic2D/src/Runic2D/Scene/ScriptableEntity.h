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

		Entity GetEntity() const { return m_Entity; }
		Scene* GetScene() const { return m_Entity.GetScene(); }
	protected:
		virtual void OnCreate() {}
		virtual void OnDestroy() {}
		virtual void OnUpdate(Timestep ts) {}

		virtual void OnCollision(Entity other) {}

		void Destroy()
		{
			m_Entity.Destroy();
		}
	private:
		Entity m_Entity;
		friend class Scene;
	};
}