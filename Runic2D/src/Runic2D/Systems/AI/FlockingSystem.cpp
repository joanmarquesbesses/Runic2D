#include "R2Dpch.h"
#include "FlockingSystem.h"

#include "Runic2D/Scene/Scene.h"
#include "Runic2D/Scene/Components/AIComponents.h"
#include "Runic2D/Scene/Components/MotionComponents.h"
#include "Runic2D/Scene/Components/CoreComponents.h"

#include "Runic2D/Core/Threading/JobSystem.h"

namespace Runic2D {

	void FlockingSystem::OnUpdate(Timestep ts, Scene* scene)
	{
		R2D_PROFILE_SCOPE("Flocking System: OnUpdate");

		auto& registry = scene->GetEntityRegistry();
		auto view = registry.view<TransformComponent, MovementComponent, FlockingComponent>();

		std::vector<entt::entity> flockEntities;
		flockEntities.reserve(view.size_hint());

		view.each([&](entt::entity e, auto&, auto&, auto&) {
			flockEntities.push_back(e);
			});

		if (flockEntities.empty()) return;

		m_SpatialHash.Clear();
		for (entt::entity entity : flockEntities)
		{
			auto& tc = registry.get<TransformComponent>(entity);
			m_SpatialHash.Insert(entity, tc.GetTranslation());
		}

		std::vector<glm::vec2> newDirections(flockEntities.size(), glm::vec2(0.0f));

		uint32_t count = (uint32_t)flockEntities.size();
		uint32_t groupSize = 16;

		auto stats = JobSystem::Dispatch(count, groupSize, [&](uint32_t start, uint32_t end)
			{
				R2D_PROFILE_SCOPE("Flocking Job");

				std::vector<entt::entity> nearbyEntities;
				nearbyEntities.reserve(64); // Bona pràctica inicial

				for (uint32_t i = start; i < end; i++)
				{
					entt::entity entity = flockEntities[i];
					auto& tc = registry.get<TransformComponent>(entity);
					auto& mc = registry.get<MovementComponent>(entity);
					auto& flock = registry.get<FlockingComponent>(entity);

					glm::vec2 myPos = tc.GetTranslation();
					glm::vec2 myTargetDir = mc.direction;

					if (glm::length(myTargetDir) < 0.01f) {
						newDirections[i] = { 0.0f, 0.0f };
						continue;
					}

					glm::vec2 separation{ 0.0f };
					glm::vec2 alignment{ 0.0f };
					glm::vec2 cohesion{ 0.0f };
					int neighbors = 0;

					m_SpatialHash.GetNearby(myPos, flock.NeighborRadius, nearbyEntities);

					for (entt::entity otherEntity : nearbyEntities)
					{
						if (entity == otherEntity) continue;

						auto& otherTc = registry.get<TransformComponent>(otherEntity);
						glm::vec2 otherPos = otherTc.GetTranslation();
						float dist = glm::distance(myPos, otherPos);

						if (dist > 0.0f && dist < flock.NeighborRadius)
						{
							glm::vec2 diff = myPos - otherPos;
							separation += glm::normalize(diff) / (dist + 0.1f);
							alignment += registry.get<MovementComponent>(otherEntity).direction;
							cohesion += otherPos;
							neighbors++;
						}
					}

					if (neighbors > 0)
					{
						alignment /= (float)neighbors;
						if (glm::length(alignment) > 0.0f) alignment = glm::normalize(alignment);

						cohesion /= (float)neighbors;
						cohesion = glm::normalize(cohesion - myPos);

						if (glm::length(separation) > 0.0f) separation = glm::normalize(separation);
					}

					glm::vec2 finalDir = (myTargetDir * flock.TargetWeight) +
						(separation * flock.SeparationWeight) +
						(alignment * flock.AlignmentWeight) +
						(cohesion * flock.CohesionWeight);

					if (glm::length(finalDir) > 0.01f) {
						newDirections[i] = glm::normalize(finalDir);
					}
				}
			});

		if (stats.GroupsDispatched > 0)
			JobSystem::Wait();

		for (size_t i = 0; i < flockEntities.size(); i++)
		{
			if (glm::length(newDirections[i]) > 0.01f) {
				auto& mc = registry.get<MovementComponent>(flockEntities[i]);
				float lerpSpeed = 10.0f * (float)ts;
				if (lerpSpeed > 1.0f) lerpSpeed = 1.0f;
				mc.direction = glm::mix(mc.direction, newDirections[i], lerpSpeed);
				if (glm::length(mc.direction) > 0.001f) mc.direction = glm::normalize(mc.direction);
			}
		}
	}
}