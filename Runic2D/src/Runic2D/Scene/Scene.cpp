#include "R2Dpch.h"
#include "Scene.h"

#include <box2d/box2d.h>

#include "Component.h"
#include "Runic2D/Renderer/Renderer2D.h"
#include "Runic2D/Math/Math.h"

#include "Entity.h"

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

				// Guardem el següent perquè quan destruïm l'actual perdrem l'enllaç
				Entity childEntity{ currentChild, this };
				if (childEntity.HasComponent<RelationshipComponent>())
					nextChild = childEntity.GetComponent<RelationshipComponent>().NextSibling;

				// Destruïm el fill
				DestroyEntity(childEntity);

				currentChild = nextChild;
			}
		}

		UnparentEntity(entity);

		m_Registry.destroy(entity);
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
				//TODO: Move to Scene::OnPlayScene
			//Script instantiation
			if (!nsc.Instance) {
				nsc.Instance = nsc.InstantiateScript();
				nsc.Instance->m_Entity = Entity{ entity, this };
				nsc.Instance->OnCreate();
			}
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
		}

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
					//Renderer2D::DrawSprite(worldTransform, sprite, (int)entityID);
					Renderer2D::DrawRect(worldTransform, sprite.Color, (int)entityID);
				});

			auto circleView = m_Registry.view<TransformComponent, CircleRendererComponent>();

			circleView.each([&](auto entityID, auto& transform, auto& circle)
				{
					Entity e{ entityID, this };
					glm::mat4 worldTransform = GetWorldTransform(transform, e);
					Renderer2D::DrawCircle(worldTransform, circle.Color, circle.Thickness, circle.Fade, (int)entityID);
				});

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
				//Renderer2D::DrawSprite(worldTransform, sprite, (int)entityID);
				Renderer2D::DrawRect(worldTransform, sprite.Color, (int)entityID);
			});

		auto circleView = m_Registry.view<TransformComponent, CircleRendererComponent>();

		circleView.each([&](auto entityID, auto& transform, auto& circle)
			{
				Entity e{ entityID, this };
				glm::mat4 worldTransform = GetWorldTransform(transform, e);
				Renderer2D::DrawCircle(worldTransform, circle.Color, circle.Thickness, circle.Fade, (int)entityID);
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
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();

			glm::mat4 wolrdTransform = GetWorldTransform(entity);

			b2BodyDef bodyDef = b2DefaultBodyDef();
			bodyDef.type = Rigidbody2DTypeToBox2D(rb2d.Type);
			bodyDef.position = { wolrdTransform[3].x, wolrdTransform[3].y };
			bodyDef.rotation = b2MakeRot(transform.Rotation.z);
			bodyDef.motionLocks.angularZ = rb2d.FixedRotation;

			b2BodyId bodyId = b2CreateBody(m_PhysicsWorld, &bodyDef);
			rb2d.RuntimeBody = bodyId;

			if (entity.HasComponent<BoxCollider2DComponent>())
			{
				auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();

				// Definició de la forma (materials)
				b2ShapeDef shapeDef = b2DefaultShapeDef();
				shapeDef.density = bc2d.Density;

				float hx = std::abs(bc2d.Size.x * transform.Scale.x) * 0.5f;
				float hy = std::abs(bc2d.Size.y * transform.Scale.y) * 0.5f;

				b2Polygon boxPolygon = b2MakeOffsetBox(hx, hy, { bc2d.Offset.x, bc2d.Offset.y }, b2MakeRot(0.0f));

				b2ShapeId shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &boxPolygon);
				b2Shape_SetFriction(shapeId, bc2d.Friction);
				b2Shape_SetRestitution(shapeId, bc2d.Restitution);
				bc2d.RuntimeShape = shapeId;

				if (rb2d.Type == Rigidbody2DComponent::BodyType::Dynamic)
				{
					b2Body_ApplyMassFromShapes(bodyId);
					b2Body_SetAwake(bodyId, true);
				}
			}
		}
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
}