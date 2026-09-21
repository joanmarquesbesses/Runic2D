#include "R2Dpch.h"
#include "LifetimeSystem.h"

#include "Runic2D/Scene/Scene.h"
#include "Runic2D/Scene/Components/CoreComponents.h"
#include "Runic2D/Scene/Entity.h"

namespace Runic2D {

	void LifetimeSystem::OnUpdate(Timestep ts, Scene* scene)
	{
		auto view = scene->GetEntityRegistry().view<LifetimeComponent>();

		for (auto entityID : view)
		{
			auto& lifetime = view.get<LifetimeComponent>(entityID);

			lifetime.TimeRemaining -= ts.GetSeconds();

			if (lifetime.TimeRemaining <= 0.0f)
			{
				scene->SubmitForDestruction(Entity{ entityID, scene });
			}
		}
	}
}