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