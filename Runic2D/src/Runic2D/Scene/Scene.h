#pragma once

#include <entt.hpp>

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

		friend class Entity;
	private:
		entt::registry m_Registry;
	};

}