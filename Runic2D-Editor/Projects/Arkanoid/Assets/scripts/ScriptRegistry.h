#pragma once

#include "Runic2D/Scene/Entity.h"
#include "Runic2D/Scene/Component.h"
#include "Paddle.h"
#include <vector>
#include <string>

namespace ScriptRegistry {

	static std::vector<std::string> GetScriptNames()
	{
		return { "Paddle", "Ball" };
	}

	// 2. La funció de Bind que ja tenies
	static void BindScript(std::string name, Runic2D::Entity entity)
	{
		if (name == "Paddle")
		{
			if (entity.HasComponent<Runic2D::NativeScriptComponent>())
				entity.RemoveComponent<Runic2D::NativeScriptComponent>();

			auto& nsc = entity.AddComponent<Runic2D::NativeScriptComponent>();
			nsc.Bind<Paddle>();
			nsc.ClassName = "Paddle";
		}
		else if (name == "Ball")
		{
			// ... lo mateix per la bola ...
		}
	}
}
