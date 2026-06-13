#include "SurvivorPCH.h"
#include "KnockbackSystem.h"

#include "Core/GameComponents.h"

using namespace Runic2D;

namespace Survivor {
	void Survivor::KnockbackSystem::OnFixedUpdate(Runic2D::Timestep ts, Runic2D::Scene* scene)
	{
		auto& registry = scene->GetEntityRegistry();
		auto view = registry.view<KnockbackComponent, Runic2D::Rigidbody2DComponent>();
		std::vector<entt::entity> entitiesToRemove;
		view.each([&](auto entity, auto& knockback, auto& rb)
			{
				knockback.TimeRemaining -= ts;
				if (knockback.TimeRemaining <= 0.0f)
				{
					if (B2_IS_NON_NULL(rb.RuntimeBody))
					{
						b2Body_SetLinearVelocity(rb.RuntimeBody, { 0.0f, 0.0f });
						int shapeCount = b2Body_GetShapeCount(rb.RuntimeBody);
						if (shapeCount > 0)
						{
							std::vector<b2ShapeId> shapes(shapeCount);
							b2Body_GetShapes(rb.RuntimeBody, shapes.data(), shapeCount);
							for (auto shapeId : shapes) {
								b2Filter filter = b2Shape_GetFilter(shapeId);
								filter.maskBits |= PhysicsLayers::Enemy; // <-- Aquí és on podries fer servir knockback.MaskToRestore
								b2Shape_SetFilter(shapeId, filter);
							}
						}
					}
					entitiesToRemove.push_back(entity);
				}
			});

		for (auto e : entitiesToRemove) {
			registry.remove<KnockbackComponent>(e);
		}
	}
}
