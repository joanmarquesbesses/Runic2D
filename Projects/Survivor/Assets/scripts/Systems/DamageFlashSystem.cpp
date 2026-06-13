#include "SurvivorPCH.h"
#include "DamageFlashSystem.h"

#include "Runic2D/Scene/Scene.h"
#include "Core/GameComponents.h"

using namespace Runic2D;

namespace Survivor {

	void DamageFlashSystem::OnUpdate(Timestep ts, Scene* scene)
	{
		auto& registry = scene->GetEntityRegistry();
		auto view = registry.view<DamageFlashComponent, SpriteRendererComponent>();
		std::vector<entt::entity> entitiesToRemove;
		view.each([&](auto entity, auto& flash, auto& sprite)
			{
				flash.TimeRemaining -= ts;
				sprite.Color = flash.FlashColor;
				if (flash.TimeRemaining <= 0.0f)
				{
					sprite.Color = glm::vec4(1.0f);
					entitiesToRemove.push_back(entity);
				}
			});

		for (auto e : entitiesToRemove) {
			registry.remove<DamageFlashComponent>(e);
		}
	}
}
