#pragma once

#include "Runic2D/Scene/Entity.h"
#include "Runic2D/Scene/Component.h"

//include here escripts to register
#include "Player.h"
#include "CameraController.h"
#include "GameManager.h"

#define REGISTER_SCRIPT(ScriptType) \
	if (name == #ScriptType) \
	{ \
		if (entity.HasComponent<Runic2D::NativeScriptComponent>()) \
			entity.RemoveComponent<Runic2D::NativeScriptComponent>(); \
		auto& nsc = entity.AddComponent<Runic2D::NativeScriptComponent>(); \
		nsc.Bind<ScriptType>(); \
		nsc.ClassName = #ScriptType; \
		return; \
	}

namespace ScriptRegistry {

	static std::vector<std::string> GetScriptNames()
	{
		return { "Player", "CameraController", "GameManager"};
	}

	static void BindScript(std::string name, Runic2D::Entity entity)
	{
		REGISTER_SCRIPT(Player);
		REGISTER_SCRIPT(CameraController);
		REGISTER_SCRIPT(GameManager);
	}
}
