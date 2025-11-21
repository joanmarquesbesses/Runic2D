#pragma once

#include <entt.hpp>

#include "Runic2D/Core/Timestep.h"

namespace Runic2D {

	class Scene {
	public:
		Scene();
		~Scene();

		//Temporary: will be replaced with a proper Entity class later
		entt::entity CreateEntity();

		//Temporary: will be replaced with a proper Entity class later
		entt::registry& Registry() { return m_Registry; }

		void OnUpdate(Timestep ts);
	private:
		entt::registry m_Registry;
	};

}