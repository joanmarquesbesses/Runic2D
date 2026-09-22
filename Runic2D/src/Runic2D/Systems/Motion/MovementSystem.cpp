#include "R2Dpch.h"
#include "MovementSystem.h"

#include "Runic2D/Scene/Scene.h"

#include "Runic2D/Scene/Components/MotionComponents.h"
#include "Runic2D/Scene/Components/PhysicsComponents.h"

namespace Runic2D {
	void MovementSystem::OnFixedUpdate(Timestep ts, Scene* scene)
	{
		auto view = scene->GetEntityRegistry().view<MovementComponent, Rigidbody2DComponent>();
		view.each(
			[&](auto entity, auto& mv, auto& rb)
			{
				b2BodyId bodyId = rb.RuntimeBody;
				b2Body_SetLinearVelocity(bodyId, { mv.direction.x * mv.speed, mv.direction.y * mv.speed });
			}
		);
	}
}

