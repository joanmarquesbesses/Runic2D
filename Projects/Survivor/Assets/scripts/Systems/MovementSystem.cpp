#include "SurvivorPCH.h"
#include "MovementSystem.h"

#include "Runic2D/Scene/Scene.h"

#include "Core/GameComponents.h"
#include "Runic2D/Scene/Components/PhysicsComponents.h"

using namespace Runic2D;

namespace Survivor {
	void MovementSystem::OnFixedUpdate(Timestep ts, Scene* scene)
	{
		auto view = scene->GetEntityRegistry().view<MovementComponent, Rigidbody2DComponent>(entt::exclude<KnockbackComponent, DeadTag>);
		view.each(
			[&](auto entity, auto& mv, auto& rb)
			{
				b2BodyId bodyId = rb.RuntimeBody;
				b2Body_SetLinearVelocity(bodyId, { mv.direction.x * mv.speed, mv.direction.y * mv.speed });
			}
		);
	}
}
