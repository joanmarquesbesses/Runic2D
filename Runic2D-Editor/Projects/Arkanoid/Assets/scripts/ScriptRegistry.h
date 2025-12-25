#pragma once

#include "Runic2D/Scene/Entity.h"
#include "Runic2D/Scene/Component.h"

#include "Paddle.h"
#include "Ball.h"
#include"Block.h"

#include <vector>
#include <string>

namespace ScriptRegistry {

	static std::vector<std::string> GetScriptNames()
	{
		return { "Paddle", "Ball", "Block"};
	}

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
			if (entity.HasComponent<Runic2D::NativeScriptComponent>())
				entity.RemoveComponent<Runic2D::NativeScriptComponent>();

			auto& nsc = entity.AddComponent<Runic2D::NativeScriptComponent>();
			nsc.Bind<Ball>();
			nsc.ClassName = "Ball";
		}
		else if (name == "Block")
		{
			if (entity.HasComponent<Runic2D::NativeScriptComponent>())
				entity.RemoveComponent<Runic2D::NativeScriptComponent>();

			auto& nsc = entity.AddComponent<Runic2D::NativeScriptComponent>();
			nsc.Bind<Block>();
			nsc.ClassName = "Block";
		}
	}
}
