#include "R2Dpch.h"
#include "Scene.h"

#include "Runic2D/Renderer/Renderer2D.h"
#include "Runic2D/Math/Math.h"

#include "Entity.h"
#include "Component.h"

namespace Runic2D {

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

	Scene::Scene()
	{
		m_Registry.on_construct<CameraComponent>().connect<&Scene::OnCameraComponentConstruct>(this);
	}

	Scene::~Scene()
	{
		m_Registry.on_construct<CameraComponent>().disconnect<&Scene::OnCameraComponentConstruct>(this);
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	template<typename Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		auto view = src.view<Component>();
		for (auto e : view)
		{
			UUID uuid = src.get<IDComponent>(e).ID;
			R2D_CORE_ASSERT(enttMap.find(uuid) != enttMap.end());
			entt::entity dstEnttID = enttMap.at(uuid);

			auto& component = src.get<Component>(e);
			dst.emplace_or_replace<Component>(dstEnttID, component);
		}
	}

	template<typename Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		if (src.HasComponent<Component>())
			dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{

	}

	Ref<Scene> Scene::Copy(Ref<Scene> other)
	{
		Ref<Scene> newScene = CreateRef<Scene>();

		newScene->m_ViewportWidth = other->m_ViewportWidth;
		newScene->m_ViewportHeight = other->m_ViewportHeight;

		auto& srcSceneRegistry = other->m_Registry;
		auto& dstSceneRegistry = newScene->m_Registry;

		std::unordered_map<UUID, entt::entity> enttMap;

		auto idView = srcSceneRegistry.view<IDComponent>();
		std::vector<entt::entity> entitiesToCopy;
		entitiesToCopy.reserve(idView.size());
		for (auto e : idView) entitiesToCopy.push_back(e);

		for (auto it = entitiesToCopy.rbegin(); it != entitiesToCopy.rend(); ++it)
		{
			entt::entity e = *it;
			UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
			const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
			Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
			enttMap[uuid] = (entt::entity)newEntity;
		}

		CopyComponent<TransformComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<SpriteRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<CircleRendererComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<CameraComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<NativeScriptComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<Rigidbody2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<BoxCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<RelationshipComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<CircleCollider2DComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<TextComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);
		CopyComponent<AnimationComponent>(dstSceneRegistry, srcSceneRegistry, enttMap);

		auto relationshipView = dstSceneRegistry.view<RelationshipComponent>();
		for (auto e : relationshipView)
		{
			auto& newRC = dstSceneRegistry.get<RelationshipComponent>(e);

			auto remapEntity = [&](entt::entity oldHandle) -> entt::entity {
				if (oldHandle == entt::null) return entt::null;
				UUID uuid = srcSceneRegistry.get<IDComponent>(oldHandle).ID;
				return enttMap.at(uuid);
				};

			newRC.Parent = remapEntity(newRC.Parent);
			newRC.NextSibling = remapEntity(newRC.NextSibling);
			newRC.PrevSibling = remapEntity(newRC.PrevSibling);
			newRC.FirstChild = remapEntity(newRC.FirstChild);
		}

		return newScene;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		if (entity.HasComponent<RelationshipComponent>())
		{
			auto& rc = entity.GetComponent<RelationshipComponent>();
			entt::entity currentChild = rc.FirstChild;
			while (currentChild != entt::null)
			{
				entt::entity nextChild = entt::null;

				Entity childEntity{ currentChild, this };
				if (childEntity.HasComponent<RelationshipComponent>())
					nextChild = childEntity.GetComponent<RelationshipComponent>().NextSibling;

				DestroyEntity(childEntity);
				currentChild = nextChild;
			}
		}

		if (entity.HasComponent<NativeScriptComponent>())
		{
			auto& nsc = entity.GetComponent<NativeScriptComponent>();

			if (nsc.Instance)
			{
				nsc.Instance->OnDestroy();
				nsc.DestroyScript(&nsc);
			}
		}

		if (entity.HasComponent<Rigidbody2DComponent>())
		{
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			if (B2_IS_NON_NULL(rb2d.RuntimeBody) && m_PhysicsWorld.index1 != 0)
			{
				b2DestroyBody(rb2d.RuntimeBody);
				rb2d.RuntimeBody = b2_nullBodyId;
			}
		}

		UnparentEntity(entity);

		m_Registry.destroy(entity);
	}

	void Scene::SubmitForDestruction(Entity entity)
	{
		m_DestructionQueue.push_back((entt::entity)entity);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };

		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		entity.AddComponent<RelationshipComponent>();

		return entity;
	}

	Entity Scene::GetEntityByUUID(UUID uuid)
	{
		auto view = m_Registry.view<IDComponent>();
		for (auto entity : view)
		{
			const auto& idComponent = view.get<IDComponent>(entity);
			if (idComponent.ID == uuid)
				return { entity, this };
		}
		return {};
	}

	void Scene::OnUpdateRunTime(Timestep ts)
	{

		//Update Scripts
		m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc) 
		{
			//Script instantiation for created entities in runtime
			if (!nsc.Instance) {
				nsc.Instance = nsc.InstantiateScript();
				nsc.Instance->m_Entity = Entity{ entity, this };
				nsc.Instance->OnCreate();
			}

			//Update Script
			if (nsc.Instance)
				nsc.Instance->OnUpdate(ts);
			
		});

		//Physics Step
		if (B2_IS_NON_NULL(m_PhysicsWorld))
		{
			b2World_Step(m_PhysicsWorld, ts, 4);

			auto view = m_Registry.view<Rigidbody2DComponent>();
			for (auto e : view)
			{
				Entity entity = { e, this };
				auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

				if (B2_IS_NULL(rb2d.RuntimeBody))
					continue;

				b2BodyId bodyId = rb2d.RuntimeBody;
				b2Body_SetAwake(bodyId, true);
				if (!b2Body_IsAwake(bodyId))
					continue;

				b2Vec2 position = b2Body_GetPosition(bodyId);
				b2Rot rotation = b2Body_GetRotation(bodyId);

				auto& transform = entity.GetComponent<TransformComponent>();
				transform.Translation.x = position.x;
				transform.Translation.y = position.y;
				transform.Rotation.z = b2Rot_GetAngle(rotation);
				transform.IsDirty = true;
			}

			b2ContactEvents events = b2World_GetContactEvents(m_PhysicsWorld);

			for (int i = 0; i < events.beginCount; ++i)
			{
				b2ContactBeginTouchEvent* event = events.beginEvents + i;

				b2ShapeId shapeA = event->shapeIdA;
				b2ShapeId shapeB = event->shapeIdB;

				uint64_t uuidA = (uintptr_t)b2Shape_GetUserData(shapeA);
				uint64_t uuidB = (uintptr_t)b2Shape_GetUserData(shapeB);

				Entity entityA = GetEntityByUUID(uuidA);
				Entity entityB = GetEntityByUUID(uuidB);

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

			b2SensorEvents sensorEvents = b2World_GetSensorEvents(m_PhysicsWorld);

			for (int i = 0; i < sensorEvents.beginCount; ++i)
			{
				b2SensorBeginTouchEvent* event = sensorEvents.beginEvents + i;

				b2ShapeId shapeA = event->sensorShapeId;
				b2ShapeId shapeB = event->visitorShapeId;

				uint64_t uuidA = (uintptr_t)b2Shape_GetUserData(shapeA);
				uint64_t uuidB = (uintptr_t)b2Shape_GetUserData(shapeB);

				Entity entityA = GetEntityByUUID(uuidA);
				Entity entityB = GetEntityByUUID(uuidB);

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
		}

		//Update Particles
		m_ParticleSystem.OnUpdate(ts);
		UpdateAnimation(ts);
		OnRenderRuntime();

		for (auto e : m_DestructionQueue)
		{
			if (m_Registry.valid(e))
			{
				Entity entity{ e, this };
				DestroyEntity(entity);
			}
		}
		m_DestructionQueue.clear();
	}

	void Scene::OnRenderRuntime()
	{
		//Find Main Camera
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		m_Registry.view<TransformComponent, CameraComponent>().each([&](auto entity, auto& transformComponent, auto& cameraComponent) {
			if (cameraComponent.Primary) {
				mainCamera = &cameraComponent.Camera;
				cameraTransform = transformComponent.GetTransform();
			}
			});

		// Render Scene
		if (mainCamera) {
			Renderer2D::BeginScene(*mainCamera, cameraTransform);

			auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();

			view.each([&](auto entityID, auto& transform, auto& sprite)
				{
					Entity e{ entityID, this };
					glm::mat4 worldTransform = GetWorldTransform(transform, e);
					Renderer2D::DrawSprite(worldTransform, sprite, (int)entityID);
				});

			auto circleView = m_Registry.view<TransformComponent, CircleRendererComponent>();

			circleView.each([&](auto entityID, auto& transform, auto& circle)
				{
					Entity e{ entityID, this };
					glm::mat4 worldTransform = GetWorldTransform(transform, e);
					Renderer2D::DrawCircle(worldTransform, circle.Color, circle.Thickness, circle.Fade, (int)entityID);
				});

			m_Registry.view<TransformComponent, TextComponent>().each([&](auto entityID, auto& transform, auto& text)
				{
					if (!text.Visible)
						return;
					Entity e{ entityID, this };
					glm::mat4 worldTransform = GetWorldTransform(transform, e);
					Renderer2D::DrawString(text.TextString, text.FontAsset, worldTransform, text.Color, text.Kerning, text.LineSpacing, (int)entityID);
				});

			m_ParticleSystem.OnRender();

			Renderer2D::EndScene();
		}
	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera);

		auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();

		view.each([&](auto entityID, auto& transform, auto& sprite)
			{
				Entity e{ entityID, this };
				glm::mat4 worldTransform = GetWorldTransform(transform, e);
				Renderer2D::DrawSprite(worldTransform, sprite, (int)entityID);
			});

		auto circleView = m_Registry.view<TransformComponent, CircleRendererComponent>();

		circleView.each([&](auto entityID, auto& transform, auto& circle)
			{
				Entity e{ entityID, this };
				glm::mat4 worldTransform = GetWorldTransform(transform, e);
				Renderer2D::DrawCircle(worldTransform, circle.Color, circle.Thickness, circle.Fade, (int)entityID);
			});

		UpdateAnimation(ts);

		// Draw Camera Bounds
		SceneCamera* mainCamera = nullptr;
		glm::mat4 cameraTransform;

		m_Registry.view<TransformComponent, CameraComponent>().each([&](auto entity, auto& tc, auto& cc)
			{
				if (cc.Primary)
				{
					mainCamera = &cc.Camera;
					cameraTransform = tc.GetTransform();
				}
			});

		if (mainCamera)
		{
			float orthoSize = mainCamera->GetOrthographicSize();
			float height = orthoSize;

			float aspectRatio = mainCamera->GetAspectRatio();
			float width = height * aspectRatio;

			glm::mat4 debugTransform = cameraTransform * glm::scale(glm::mat4(1.0f), { width, height, 1.0f });

			Renderer2D::DrawRect(debugTransform, { 0.0f, 1.0f, 0.0f, 1.0f });
		}

		m_Registry.view<TransformComponent, TextComponent>().each([&](auto entityID, auto& transform, auto& text)
			{
				Entity e{ entityID, this };
				glm::mat4 worldTransform = GetWorldTransform(transform, e);
				Renderer2D::DrawString(text.TextString, text.FontAsset, worldTransform, text.Color, text.Kerning, text.LineSpacing, (int)entityID);
			});

		Renderer2D::EndScene();
	}

	void Scene::OnRuntimeStart()
	{
		b2WorldDef worldDef = b2DefaultWorldDef();
		worldDef.gravity = { 0.0f, -9.8f }; 
		m_PhysicsWorld = b2CreateWorld(&worldDef);

		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			InstantiatePhysics(entity);
		}

		m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
			{
				//Script instantiation
				if (!nsc.Instance) {
					nsc.Instance = nsc.InstantiateScript();
					nsc.Instance->m_Entity = Entity{ entity, this };
					nsc.Instance->OnCreate();
				}
			});

		m_Registry.view<AnimationComponent>().each([=](auto entity, auto& anim)
			{
				anim.Animations.clear();

				for (auto& profile : anim.Profiles)
				{
					if (profile.AtlasTexture)
					{
						int numCols = (int)(profile.AtlasTexture->GetWidth() / profile.TileSize.x);
						int col = profile.StartFrame % numCols;
						int row = profile.StartFrame / numCols;

						float startX = col * profile.TileSize.x;
						float startY = row * profile.TileSize.y;

						Ref<Animation2D> animAsset = Animation2D::CreateFromAtlas(
							profile.AtlasTexture,
							profile.TileSize,
							{ startX, startY },
							profile.FrameCount,
							profile.FramesPerRow,
							profile.FrameTime
						);

						anim.Animations[profile.Name] = animAsset;
					}
				}

				if (!anim.Animations.empty())
				{
					if (!anim.CurrentStateName.empty() && anim.Animations.find(anim.CurrentStateName) != anim.Animations.end())
					{
						anim.CurrentAnimation = anim.Animations[anim.CurrentStateName];
					}
					else if (anim.Animations.find("Idle") != anim.Animations.end())
					{
						anim.CurrentAnimation = anim.Animations["Idle"];
						anim.CurrentStateName = "Idle";
					}
					else
					{
						anim.CurrentAnimation = anim.Animations.begin()->second;
						anim.CurrentStateName = anim.Animations.begin()->first;
					}

					anim.Playing = true;
				}
			});
	}

	void Scene::OnRuntimeStop()
	{
		if (B2_IS_NON_NULL(m_PhysicsWorld))
		{
			b2DestroyWorld(m_PhysicsWorld);
			m_PhysicsWorld = b2_nullWorldId;
		}

		auto view = m_Registry.view<Rigidbody2DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
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

		//Destroy script instances
		m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
			{
				if (nsc.Instance)
				{
					nsc.Instance->OnDestroy();
					nsc.DestroyScript(&nsc);  
					nsc.Instance = nullptr;    
				}
			});
	}

	void Scene::InstantiatePhysics(Entity entity)
	{
		auto& transform = entity.GetComponent<TransformComponent>();
		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

		glm::mat4 wolrdTransform = GetWorldTransform(entity);

		b2BodyDef bodyDef = b2DefaultBodyDef();
		bodyDef.type = Rigidbody2DTypeToBox2D(rb2d.Type);
		bodyDef.position = { wolrdTransform[3].x, wolrdTransform[3].y };
		bodyDef.rotation = b2MakeRot(transform.Rotation.z);
		bodyDef.enableSleep = true;
		bodyDef.motionLocks.angularZ = rb2d.FixedRotation;
		bodyDef.gravityScale = rb2d.GravityScale;

		b2BodyId bodyId = b2CreateBody(m_PhysicsWorld, &bodyDef);
		rb2d.RuntimeBody = bodyId;

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

			float hx = std::abs(bc2d.Size.x * transform.Scale.x) * 0.5f;
			float hy = std::abs(bc2d.Size.y * transform.Scale.y) * 0.5f;

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

			float maxScale = std::max(transform.Scale.x, transform.Scale.y);
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

	void Scene::UpdateEntityColliders(Entity entity)
	{
		if (!entity.HasComponent<Rigidbody2DComponent>()) return;

		auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
		auto& transform = entity.GetComponent<TransformComponent>();

		if (B2_IS_NULL(rb2d.RuntimeBody)) return;

		b2BodyId bodyId = rb2d.RuntimeBody;

		if (entity.HasComponent<BoxCollider2DComponent>())
		{
			auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
			if (B2_IS_NON_NULL(bc2d.RuntimeShape))
			{
				b2DestroyShape(bc2d.RuntimeShape, false);
			}

			b2ShapeDef shapeDef = b2DefaultShapeDef();
			shapeDef.density = bc2d.Density;
			shapeDef.userData = (void*)(uintptr_t)entity.GetUUID();
			shapeDef.filter.categoryBits = bc2d.CategoryBits; 
			shapeDef.filter.maskBits = bc2d.MaskBits;         
			shapeDef.filter.groupIndex = bc2d.GroupIndex;
			shapeDef.isSensor = bc2d.IsSensor;
			shapeDef.enableSensorEvents = bc2d.EnableSensorEvents;
			shapeDef.enableContactEvents = bc2d.EnableContactEvents;

			float hx = std::abs(bc2d.Size.x * transform.Scale.x) * 0.5f;
			float hy = std::abs(bc2d.Size.y * transform.Scale.y) * 0.5f;

			b2Polygon boxPolygon = b2MakeOffsetBox(hx, hy, { bc2d.Offset.x, bc2d.Offset.y }, b2MakeRot(0.0f));

			b2ShapeId shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &boxPolygon);

			b2Shape_SetFriction(shapeId, bc2d.Friction);
			b2Shape_SetRestitution(shapeId, bc2d.Restitution);

			bc2d.RuntimeShape = shapeId;
		}

		if (entity.HasComponent<CircleCollider2DComponent>())
		{
			auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();

			if (B2_IS_NON_NULL(cc2d.RuntimeShape))
			{
				b2DestroyShape(cc2d.RuntimeShape, false);
			}

			b2ShapeDef shapeDef = b2DefaultShapeDef();
			shapeDef.density = cc2d.Density;
			shapeDef.userData = (void*)(uintptr_t)entity.GetUUID();
			shapeDef.filter.categoryBits = cc2d.CategoryBits;
			shapeDef.filter.maskBits = cc2d.MaskBits;
			shapeDef.filter.groupIndex = cc2d.GroupIndex;
			shapeDef.isSensor = cc2d.IsSensor;
			shapeDef.enableSensorEvents = cc2d.EnableSensorEvents;
			shapeDef.enableContactEvents = cc2d.EnableContactEvents;

			float maxScale = std::max(std::abs(transform.Scale.x), std::abs(transform.Scale.y));
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
		}
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportHeight = height;
		m_ViewportWidth = width;

		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view) {
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio) {
				cameraComponent.Camera.SetViewportSize(width, height);
			}
		}
	}

	void Scene::DuplicateEntity(Entity entity)
	{
		Entity newEntity = CreateEntity(entity.GetComponent<TagComponent>().Tag);

		CopyEntity(entity, newEntity);

		if (entity.HasComponent<RelationshipComponent>())
		{
			auto& rc = entity.GetComponent<RelationshipComponent>();
			if (rc.Parent != entt::null)
			{
				Entity parent = { rc.Parent, this };

				// 1. Configurem el component del fill
				auto& childRC = newEntity.GetOrAddComponent<RelationshipComponent>();
				childRC.Parent = rc.Parent;

				// 2. Configurem el component del pare
				auto& parentRC = parent.GetComponent<RelationshipComponent>(); // El pare ja té component segur

				// 3. Afegim al final de la llista de fills del pare
				if (parentRC.FirstChild == entt::null)
				{
					parentRC.FirstChild = (entt::entity)newEntity;
				}
				else
				{
					// Busquem l'últim germà
					entt::entity prevNode = parentRC.FirstChild;
					while (true)
					{
						Entity prevEntity{ prevNode, this };
						auto& prevRC = prevEntity.GetComponent<RelationshipComponent>();
						if (prevRC.NextSibling == entt::null)
						{
							prevRC.NextSibling = (entt::entity)newEntity;
							childRC.PrevSibling = prevNode;
							break;
						}
						prevNode = prevRC.NextSibling;
					}
				}
				parentRC.ChildrenCount++;
			}
		}

		std::function<void(Entity, Entity)> CopyChildrenRec = [&](Entity srcParent, Entity dstParent)
			{
				if (!srcParent.HasComponent<RelationshipComponent>())
					return;

				auto& srcRC = srcParent.GetComponent<RelationshipComponent>();
				entt::entity currentChildHandle = srcRC.FirstChild;

				while (currentChildHandle != entt::null)
				{
					Entity srcChild = { currentChildHandle, this };
					Entity dstChild = CreateEntity(srcChild.GetComponent<TagComponent>().Tag);

					CopyEntity(srcChild, dstChild);

					auto& childRC = dstChild.GetOrAddComponent<RelationshipComponent>();
					childRC.Parent = (entt::entity)dstParent;

					auto& parentRC = dstParent.GetOrAddComponent<RelationshipComponent>();

					if (parentRC.FirstChild == entt::null)
					{
						parentRC.FirstChild = (entt::entity)dstChild;
					}
					else
					{
						entt::entity prevNode = parentRC.FirstChild;
						while (true)
						{
							Entity prevEntity{ prevNode, this };
							auto& prevRC = prevEntity.GetComponent<RelationshipComponent>();
							if (prevRC.NextSibling == entt::null)
							{
								prevRC.NextSibling = (entt::entity)dstChild;
								childRC.PrevSibling = prevNode;
								break;
							}
							prevNode = prevRC.NextSibling;
						}
					}
					parentRC.ChildrenCount++;

					CopyChildrenRec(srcChild, dstChild);

					currentChildHandle = srcChild.GetComponent<RelationshipComponent>().NextSibling;
				}
			};

		CopyChildrenRec(entity, newEntity);
	}

	void Scene::CopyEntity(Entity src, Entity dst)
	{
		CopyComponentIfExists<TransformComponent>(dst, src);
		CopyComponentIfExists<SpriteRendererComponent>(dst, src);
		CopyComponentIfExists<CircleRendererComponent>(dst, src);
		CopyComponentIfExists<CameraComponent>(dst, src);
		CopyComponentIfExists<NativeScriptComponent>(dst, src);
		CopyComponentIfExists<Rigidbody2DComponent>(dst, src);
		CopyComponentIfExists<BoxCollider2DComponent>(dst, src);
		CopyComponentIfExists<CircleCollider2DComponent>(dst, src);
		CopyComponentIfExists<TextComponent>(dst, src);
		CopyComponentIfExists<AnimationComponent>(dst, src);
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view) {
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (cameraComponent.Primary) {
				return Entity{ entity, this };
			}
		}
		return Entity{};
	}

	void Scene::SetCollisionEnabled(Entity entity, bool enabled)
	{
		if (!entity.HasComponent<BoxCollider2DComponent>()) return;

		auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

		if (b2Shape_IsValid(bc2d.RuntimeShape))
		{
			b2Filter filter = b2Shape_GetFilter(bc2d.RuntimeShape);

			filter.maskBits = enabled ? 0xFFFFFFFF : 0x00000000;

			b2Shape_SetFilter(bc2d.RuntimeShape, filter);
		}
	}

	void Scene::OnCameraComponentConstruct(entt::registry& registry, entt::entity entity)
	{
		auto& component = registry.get<CameraComponent>(entity);

		if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
			component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
	}

	void Scene::ParentEntity(Entity entity, Entity parent)
	{
		if (entity == parent) return;

		glm::mat4 oldWorldTransform = GetWorldTransform(entity);

		UnparentEntity(entity);

		auto& childRC = entity.GetComponent<RelationshipComponent>();
		childRC.Parent = (entt::entity)parent;

		auto& parentRC = parent.GetComponent<RelationshipComponent>();

		if (parentRC.FirstChild == entt::null)
		{
			parentRC.FirstChild = (entt::entity)entity;
		}
		else
		{
			entt::entity prevNode = parentRC.FirstChild;
			while (true)
			{
				Entity prevEntity{ prevNode, this };
				auto& prevRC = prevEntity.GetComponent<RelationshipComponent>();

				if (prevRC.NextSibling == entt::null)
				{
					prevRC.NextSibling = (entt::entity)entity;
					childRC.PrevSibling = prevNode;
					break;
				}
				prevNode = prevRC.NextSibling;
			}
		}

		parentRC.ChildrenCount++;

		glm::mat4 parentWorldTransform = GetWorldTransform(parent);
		glm::mat4 newLocalTransform = glm::inverse(parentWorldTransform) * oldWorldTransform;

		auto& tc = entity.GetComponent<TransformComponent>();
		Math::DecomposeTransform(newLocalTransform, tc.Translation, tc.Rotation, tc.Scale);
		tc.IsDirty = true;
	}

	void Scene::UnparentEntity(Entity entity, bool convertToWorldSpace)
	{
		auto& childRC = entity.GetComponent<RelationshipComponent>();
		if (childRC.Parent == entt::null)
			return;

		glm::mat4 worldTransform = glm::mat4(1.0f);
		if (convertToWorldSpace) {
			worldTransform = GetWorldTransform(entity);
		}

		Entity parent{ childRC.Parent, this };
		auto& parentRC = parent.GetComponent<RelationshipComponent>();

		if (childRC.PrevSibling != entt::null)
		{
			Entity prevEntity{ childRC.PrevSibling, this };
			prevEntity.GetComponent<RelationshipComponent>().NextSibling = childRC.NextSibling;
		}

		if (childRC.NextSibling != entt::null)
		{
			Entity nextEntity{ childRC.NextSibling, this };
			nextEntity.GetComponent<RelationshipComponent>().PrevSibling = childRC.PrevSibling;
		}

		if (parentRC.FirstChild == (entt::entity)entity)
		{
			parentRC.FirstChild = childRC.NextSibling;
		}

		parentRC.ChildrenCount--;

		childRC.Parent = entt::null;
		childRC.NextSibling = entt::null;
		childRC.PrevSibling = entt::null;

		if (convertToWorldSpace) {
			auto& tc = entity.GetComponent<TransformComponent>();
			Math::DecomposeTransform(worldTransform, tc.Translation, tc.Rotation, tc.Scale);
			tc.IsDirty = true;
		}
	}

	Entity Scene::FindEntityByName(std::string_view name)
	{
		auto view = m_Registry.view<TagComponent>();
		for (auto entityID : view)
		{
			const auto& tc = view.get<TagComponent>(entityID);
			if (tc.Tag == name)
				return Entity{ entityID, this };
		}
		return {};
	}

	glm::mat4 Scene::GetWorldTransform(Entity entity)
	{
		glm::mat4 transform = entity.GetComponent<TransformComponent>().GetTransform();

		if (entity.HasComponent<RelationshipComponent>())
		{
			entt::entity parentID = entity.GetComponent<RelationshipComponent>().Parent;
			if (parentID != entt::null)
			{
				Entity parent{ parentID, this };
				transform = GetWorldTransform(parent) * transform;
			}
		}

		return transform;
	}

	glm::mat4 Scene::GetWorldTransform(const TransformComponent& transform, Entity entity)
	{
		glm::mat4 worldTransform = transform.GetTransform();

		if (entity.HasComponent<RelationshipComponent>())
		{
			entt::entity parentID = entity.GetComponent<RelationshipComponent>().Parent;
			if (parentID != entt::null)
			{
				Entity parent{ parentID, this };
				worldTransform = GetWorldTransform(parent) * worldTransform;
			}
		}

		return worldTransform;
	}

	void Scene::OnRenderOverlay(const glm::mat4& viewProjection)
	{
		Renderer2D::BeginScene(viewProjection);
		
		auto viewbc = m_Registry.view<TransformComponent, BoxCollider2DComponent>();

		viewbc.each([&](auto entity, auto& tc, auto& bc2d)
			{
				glm::vec3 scale = tc.Scale * glm::vec3(bc2d.Size, 1.0f);

				glm::mat4 transform = glm::translate(glm::mat4(1.0f), tc.Translation)
					* glm::rotate(glm::mat4(1.0f), tc.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
					* glm::translate(glm::mat4(1.0f), glm::vec3(bc2d.Offset, 0.001f))
					* glm::scale(glm::mat4(1.0f), scale);

				Renderer2D::DrawRect(transform, glm::vec4(0, 1, 0, 1));
			});
		
		auto viewcc = m_Registry.view<TransformComponent, CircleCollider2DComponent>();

		viewcc.each([&](auto entity, auto& tc, auto& cc2d)
			{
				float scale = std::max(tc.Scale.x, tc.Scale.y) * cc2d.Radius * 2.0f;

				glm::mat4 transform = glm::translate(glm::mat4(1.0f), tc.Translation)
					* glm::rotate(glm::mat4(1.0f), tc.Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f))
					* glm::translate(glm::mat4(1.0f), glm::vec3(cc2d.Offset, 0.001f))
					* glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, 1.0f));

				Renderer2D::DrawCircle(transform, glm::vec4(0, 1, 0, 1), 0.05f, 0.01f, (int)entity);
			});

		Renderer2D::EndScene();
	}

	void Scene::UpdateAnimation(Timestep ts)
	{
		m_Registry.view<AnimationComponent, SpriteRendererComponent>().each([&](auto entityID, auto& anim, auto& sprite)
			{
				if (anim.Animations.empty() && !anim.Profiles.empty())
				{
					for (auto& profile : anim.Profiles)
					{
						if (profile.AtlasTexture)
						{
							int numCols = (int)(profile.AtlasTexture->GetWidth() / profile.TileSize.x);
							int col = profile.StartFrame % numCols;
							int row = profile.StartFrame / numCols;
							float startX = (float)col * profile.TileSize.x;
							float startY = (float)row * profile.TileSize.y;

							Ref<Animation2D> animAsset = Animation2D::CreateFromAtlas(
								profile.AtlasTexture, profile.TileSize, { startX, startY },
								profile.FrameCount, profile.FramesPerRow, profile.FrameTime
							);
							anim.Animations[profile.Name] = animAsset;
						}
					}

					if (!anim.Animations.empty() && !anim.CurrentAnimation) {
						anim.CurrentAnimation = anim.Animations.begin()->second;
						anim.CurrentStateName = anim.Animations.begin()->first;
					}
				}

				if (anim.CurrentAnimation && anim.Playing)
				{
					anim.TimeAccumulator += ts;
					if (anim.TimeAccumulator >= anim.CurrentAnimation->GetFrameTime())
					{
						anim.TimeAccumulator -= anim.CurrentAnimation->GetFrameTime();
						anim.CurrentFrameIndex++;

						if (anim.CurrentFrameIndex >= anim.CurrentAnimation->GetFrameCount())
						{
							if (anim.Loop) anim.CurrentFrameIndex = 0;
							else {
								anim.CurrentFrameIndex = anim.CurrentAnimation->GetFrameCount() - 1;
								anim.Playing = false;
							}
						}

						sprite.SubTexture = anim.CurrentAnimation->GetFrame(anim.CurrentFrameIndex);
					}
				}
			});
	}
}