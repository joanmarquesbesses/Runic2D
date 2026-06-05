#include "R2Dpch.h"
#include "PhysicsSystem.h"

#include "Runic2D/Scene/Entity.h"
#include "Runic2D/Scene/Component.h"

#include "Runic2D/Core/Application.h"
#include "Runic2D/Core/JobSystem.h"

#include <box2d/types.h>

namespace Runic2D {

	static void* Box2D_EnqueueTask(b2TaskCallback* task, int itemCount, int minRange, void* taskContext, void* userContext)
	{
		R2D_PROFILE_FUNCTION();
		JobSystem::Dispatch(
			(uint32_t)itemCount, (uint32_t)minRange, [task, taskContext](uint32_t start, uint32_t end)
			{
				R2D_PROFILE_SCOPE("Box2D Task");
				task((int)start, (int)end, JobSystem::GetThreadIndex(), taskContext);
			}
		);
		return (void*)1;
	}

	static void Box2D_FinishTask(void* userTask, void* userContext)
	{
		JobSystem::Wait();
	}

	static b2BodyType Rigidbody2DTypeToBox2D(Rigidbody2DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
			case Rigidbody2DComponent::BodyType::Static:    return b2_staticBody;
			case Rigidbody2DComponent::BodyType::Dynamic:   return b2_dynamicBody;
			case Rigidbody2DComponent::BodyType::Kinematic: return b2_kinematicBody;
		}
		return b2_staticBody;
	}

	void PhysicsSystem::OnRigidbodyDestroyed(entt::registry& registry, entt::entity entity)
	{
		auto& rb2d = registry.get<Rigidbody2DComponent>(entity);
		if (B2_IS_NON_NULL(rb2d.RuntimeBody)) {
			b2DestroyBody(rb2d.RuntimeBody);
		}
	}

	void PhysicsSystem::InstantiatePhysics(Entity entity, Scene* scene)
	{
		auto& transform = entity.GetComponent<TransformComponent>();
		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

		glm::mat4 wolrdTransform = entity.GetWorldTransform();

		b2BodyDef bodyDef = b2DefaultBodyDef();
		bodyDef.type = Rigidbody2DTypeToBox2D(rb2d.Type);
		bodyDef.position = { wolrdTransform[3].x, wolrdTransform[3].y };
		bodyDef.rotation = b2MakeRot(transform.GetRotation().z);
		bodyDef.enableSleep = true;
		bodyDef.motionLocks.angularZ = rb2d.FixedRotation;
		bodyDef.gravityScale = rb2d.GravityScale;

		b2BodyId bodyId = b2CreateBody(scene->GetPhysicsWorldID(), &bodyDef);
		rb2d.RuntimeBody = bodyId;

		b2Vec2 spawnPos = b2Body_GetPosition(bodyId);
		rb2d.PreviousTranslation = { spawnPos.x, spawnPos.y };
		rb2d.PreviousRotation = b2Rot_GetAngle(b2Body_GetRotation(bodyId));
		rb2d.InterpolationInitialized = true;

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

			b2ShapeDef shapeDef = b2DefaultShapeDef();
			shapeDef.density = bc2d.Density;

			shapeDef.userData = (void*)(uintptr_t)entity.GetUUID();
			shapeDef.isSensor = bc2d.IsSensor;
			shapeDef.filter.categoryBits = bc2d.CategoryBits;
			shapeDef.filter.maskBits = bc2d.MaskBits;
			shapeDef.filter.groupIndex = bc2d.GroupIndex;
			shapeDef.enableSensorEvents = bc2d.EnableSensorEvents;
			shapeDef.enableContactEvents = bc2d.EnableContactEvents;

			float hx = std::abs(bc2d.Size.x * transform.GetScale().x) * 0.5f;
			float hy = std::abs(bc2d.Size.y * transform.GetScale().y) * 0.5f;

			b2Polygon boxPolygon = b2MakeOffsetBox(hx, hy, { bc2d.Offset.x, bc2d.Offset.y }, b2MakeRot(0.0f));

			b2ShapeId shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &boxPolygon);
			b2Shape_SetFriction(shapeId, bc2d.Friction);
			b2Shape_SetRestitution(shapeId, bc2d.Restitution);
			bc2d.RuntimeShape = shapeId;
		}

		if (entity.HasComponent<CircleCollider2DComponent>())
		{
			auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();

			b2ShapeDef shapeDef = b2DefaultShapeDef();
			shapeDef.density = cc2d.Density;

			shapeDef.userData = (void*)(uintptr_t)entity.GetUUID();
			shapeDef.isSensor = cc2d.IsSensor;
			shapeDef.filter.categoryBits = cc2d.CategoryBits;
			shapeDef.filter.maskBits = cc2d.MaskBits;
			shapeDef.filter.groupIndex = cc2d.GroupIndex;
			shapeDef.enableSensorEvents = cc2d.EnableSensorEvents;
			shapeDef.enableContactEvents = cc2d.EnableContactEvents;

			float maxScale = std::max(transform.GetScale().x, transform.GetScale().y);
			float radius = cc2d.Radius * maxScale;

			b2Circle circle;
			circle.center = { cc2d.Offset.x, cc2d.Offset.y };
			circle.radius = radius;

			b2ShapeId shapeId = b2CreateCircleShape(bodyId, &shapeDef, &circle);
			b2Shape_SetFriction(shapeId, cc2d.Friction);
			b2Shape_SetRestitution(shapeId, cc2d.Restitution);
			cc2d.RuntimeShape = shapeId;
		}

		if (rb2d.Type == Rigidbody2DComponent::BodyType::Dynamic)
		{
			b2Body_ApplyMassFromShapes(bodyId);
			b2Body_SetAwake(bodyId, true);
		}
	}

	void PhysicsSystem::OnStart(Scene* scene)
	{
		b2WorldDef worldDef = b2DefaultWorldDef();
		worldDef.gravity = { 0.0f, -9.8f };

		// Connect JobSystem
		worldDef.workerCount = JobSystem::GetThreadCount();
		worldDef.enqueueTask = Box2D_EnqueueTask;
		worldDef.finishTask = Box2D_FinishTask;
		worldDef.userTaskContext = scene;

		scene->GetPhysicsWorldID() = b2CreateWorld(&worldDef);

		auto view = scene->GetEntityRegistry().view<Rigidbody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, scene };
			InstantiatePhysics(entity, scene);
		}

		scene->GetEntityRegistry().on_destroy<Rigidbody2DComponent>().connect<&PhysicsSystem::OnRigidbodyDestroyed>(this);
	}

	void PhysicsSystem::OnStop(Scene* scene) {

		scene->GetEntityRegistry().on_destroy<Rigidbody2DComponent>().disconnect<&PhysicsSystem::OnRigidbodyDestroyed>(this);

		if (B2_IS_NON_NULL(scene->GetPhysicsWorldID()))
		{
			b2WorldId& PhysicsWorld = scene->GetPhysicsWorldID();
			b2DestroyWorld(PhysicsWorld);
			PhysicsWorld = b2_nullWorldId;
		}

		auto view = scene->GetEntityRegistry().view<Rigidbody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, scene };
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
			rb2d.RuntimeBody = b2_nullBodyId;

			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
				bc2d.RuntimeShape = b2_nullShapeId;
			}

			if (entity.HasComponent<CircleCollider2DComponent>())
			{
				auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
				cc2d.RuntimeShape = b2_nullShapeId;
			}
		}
	}

	void PhysicsSystem::OnUpdate(Timestep ts, Scene* scene)
	{
		if (!scene->IsPaused() && B2_IS_NON_NULL(scene->GetPhysicsWorldID()))
		{
			float alpha = Application::Get().GetFixedUpdateAlpha();
			auto& registry = scene->GetEntityRegistry();

			auto view = registry.view<TransformComponent, Rigidbody2DComponent>();
			std::vector<entt::entity> entities(view.begin(), view.end());
			uint32_t count = (uint32_t)entities.size();
			if (count == 0) return;

			uint32_t groupSize = 64;

			auto stats = JobSystem::Dispatch(count, groupSize, [&registry, &entities, alpha](uint32_t start, uint32_t end)
				{
					for (uint32_t i = start; i < end; i++)
					{
						entt::entity entity = entities[i];
						auto& transform = registry.get<TransformComponent>(entity);
						auto& rb = registry.get<Rigidbody2DComponent>(entity);
						if (B2_IS_NON_NULL(rb.RuntimeBody))
						{
							b2Vec2 currentPos = b2Body_GetPosition(rb.RuntimeBody);
							float currentRot = b2Rot_GetAngle(b2Body_GetRotation(rb.RuntimeBody));

							float newX = glm::mix(rb.PreviousTranslation.x, currentPos.x, alpha);
							float newY = glm::mix(rb.PreviousTranslation.y, currentPos.y, alpha);
							transform.SetTranslation({ newX, newY, transform.GetTranslation().z });

							float newRotZ = glm::mix(rb.PreviousRotation, currentRot, alpha);
							transform.SetRotation({ transform.GetRotation().x, transform.GetRotation().y, newRotZ });
						}
					}
				});

			if (stats.GroupsDispatched > 0)
			{
				JobSystem::Wait();
			}
		}
	}

	void PhysicsSystem::OnFixedUpdate(Timestep ts, Scene* scene)
	{
		if (B2_IS_NON_NULL(scene->GetPhysicsWorldID()))
		{
			b2WorldId& PhysicsWorld = scene->GetPhysicsWorldID();
			// Store previous state for interpolation before stepping the world
			auto rbView = scene->GetEntityRegistry().view<Rigidbody2DComponent>();
			for (auto e : rbView)
			{
				auto& rb2d = rbView.get<Rigidbody2DComponent>(e);
				if (B2_IS_NON_NULL(rb2d.RuntimeBody) && rb2d.InterpolationInitialized)
				{
					b2Vec2 pos = b2Body_GetPosition(rb2d.RuntimeBody);
					rb2d.PreviousTranslation = { pos.x, pos.y };
					rb2d.PreviousRotation = b2Rot_GetAngle(b2Body_GetRotation(rb2d.RuntimeBody));
				}
			}

			b2World_Step(PhysicsWorld, ts, 4);

			b2ContactEvents events = b2World_GetContactEvents(PhysicsWorld);

			for (int i = 0; i < events.beginCount; ++i)
			{
				b2ContactBeginTouchEvent* event = events.beginEvents + i;

				b2ShapeId shapeA = event->shapeIdA;
				b2ShapeId shapeB = event->shapeIdB;

				uint64_t uuidA = (uintptr_t)b2Shape_GetUserData(shapeA);
				uint64_t uuidB = (uintptr_t)b2Shape_GetUserData(shapeB);

				Entity entityA = scene->GetEntityByUUID(uuidA);
				Entity entityB = scene->GetEntityByUUID(uuidB);

				if (entityA && entityA.HasComponent<NativeScriptComponent>())
				{
					auto& script = entityA.GetComponent<NativeScriptComponent>();
					if (script.Instance) script.Instance->OnCollision(entityB);
				}

				if (entityB && entityB.HasComponent<NativeScriptComponent>())
				{
					auto& script = entityB.GetComponent<NativeScriptComponent>();
					if (script.Instance) script.Instance->OnCollision(entityA);
				}
			}

			b2SensorEvents sensorEvents = b2World_GetSensorEvents(PhysicsWorld);

			for (int i = 0; i < sensorEvents.beginCount; ++i)
			{
				b2SensorBeginTouchEvent* event = sensorEvents.beginEvents + i;

				b2ShapeId shapeA = event->sensorShapeId;
				b2ShapeId shapeB = event->visitorShapeId;

				uint64_t uuidA = (uintptr_t)b2Shape_GetUserData(shapeA);
				uint64_t uuidB = (uintptr_t)b2Shape_GetUserData(shapeB);

				Entity entityA = scene->GetEntityByUUID(uuidA);
				Entity entityB = scene->GetEntityByUUID(uuidB);

				if (entityA && entityA.HasComponent<NativeScriptComponent>())
				{
					auto& script = entityA.GetComponent<NativeScriptComponent>();
					if (script.Instance) script.Instance->OnSensor(entityB);
				}

				if (entityB && entityB.HasComponent<NativeScriptComponent>())
				{
					auto& script = entityB.GetComponent<NativeScriptComponent>();
					if (script.Instance) script.Instance->OnSensor(entityA);
				}
			}
		}
	}
}
