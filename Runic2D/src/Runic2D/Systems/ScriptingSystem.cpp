#include "R2Dpch.h"
#include "ScriptingSystem.h"

#include "Runic2D/Scene/Components/ScriptingComponents.h"

namespace Runic2D {

	void ScriptingSystem::OnStart(Scene* scene)
	{
		scene->GetEntityRegistry().view<NativeScriptComponent>().each(
			[=](auto entity, auto& nsc)
			{
				if (!nsc.Instance) {
					nsc.Instance = nsc.InstantiateScript();
					nsc.Instance->m_Entity = Entity{ entity, scene };
					nsc.Instance->OnCreate();
				}
			}
		);

		scene->GetEntityRegistry().on_destroy<NativeScriptComponent>().connect<&ScriptingSystem::OnNativeScriptDestroyed>(this);
	}

	void ScriptingSystem::OnStop(Scene* scene)
	{
		scene->GetEntityRegistry().view<NativeScriptComponent>().each(
			[=](auto entity, auto& nsc)
			{
				if (nsc.Instance)
				{
					nsc.Instance->OnDestroy();
					nsc.DestroyScript(&nsc);
					nsc.Instance = nullptr;
				}
			}
		);

		scene->GetEntityRegistry().on_destroy<NativeScriptComponent>().disconnect<&ScriptingSystem::OnNativeScriptDestroyed>(this);
	}

	void ScriptingSystem::OnUpdate(Timestep ts, Scene* scene)
	{
		R2D_PROFILE_SCOPE("Scripting System: OnUpdate");

		scene->GetEntityRegistry().view<NativeScriptComponent>().each(
			[=](auto entity, auto& nsc)
			{
				if (!nsc.Instance) {
					nsc.Instance = nsc.InstantiateScript();
					nsc.Instance->m_Entity = Entity{ entity, scene };
					nsc.Instance->OnCreate();
				}

				if (nsc.Instance)
				{
					if (!scene->IsPaused() || nsc.Instance->UpdateWhenPaused())
						nsc.Instance->OnUpdate(ts);
				}
			}
		);
	}

	void ScriptingSystem::OnFixedUpdate(Timestep ts, Scene* scene)
	{
		R2D_PROFILE_SCOPE("Scripting System: OnFixedUpdate");

		scene->GetEntityRegistry().view<NativeScriptComponent>().each(
			[=](auto entity, auto& nsc)
			{
				if (!nsc.Instance) {
					nsc.Instance = nsc.InstantiateScript();
					nsc.Instance->m_Entity = Entity{ entity, scene };
					nsc.Instance->OnCreate();
				}

				if (nsc.Instance)
				{
					if (!scene->IsPaused() || nsc.Instance->UpdateWhenPaused())
						nsc.Instance->OnFixedUpdate(ts);
				}
			}
		);
	}

	void ScriptingSystem::OnNativeScriptDestroyed(entt::registry& registry, entt::entity entity)
	{
		auto& nsc = registry.get<NativeScriptComponent>(entity);

		if (nsc.Instance)
		{
			nsc.Instance->OnDestroy();
			nsc.DestroyScript(&nsc);
			nsc.Instance = nullptr;
		}
	}
}
