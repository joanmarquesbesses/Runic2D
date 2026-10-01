#include "R2Dpch.h"
#include "MovementSystem.h"

#include "Runic2D/Scene/Scene.h"

#include "Runic2D/Scene/Components/MotionComponents.h"
#include "Runic2D/Scene/Components/PhysicsComponents.h"
#include "Runic2D/Scene/Components/RenderComponents.h"

namespace Runic2D {
	void MovementSystem::OnFixedUpdate(Timestep ts, Scene* scene)
	{
		auto& registry = scene->GetEntityRegistry();
		auto view = registry.view<MovementComponent, Rigidbody2DComponent>();
		view.each(
			[&](auto entity, auto& mv, auto& rb)
			{
				b2BodyId bodyId = rb.RuntimeBody;
				b2Body_SetLinearVelocity(bodyId, { mv.direction.x * mv.speed, mv.direction.y * mv.speed });

				if (auto* sprite = registry.try_get<SpriteRendererComponent>(entity))
				{
					if (mv.AutoFlipVisuals) {
						if (mv.direction.x > 0.01f)
							sprite->FlipX = true;
						else if (mv.direction.x < -0.01f)
							sprite->FlipX = false;
					}
				}
			}
		);
	}
}

