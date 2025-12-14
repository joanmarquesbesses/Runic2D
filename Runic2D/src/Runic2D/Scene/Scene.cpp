#include "R2Dpch.h"
#include "Scene.h"

#include "Component.h"
#include "Runic2D/Renderer/Renderer2D.h"
#include "Runic2D/Math/Math.h"

#include "Entity.h"

namespace Runic2D {

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

		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		//Render Sprites
		m_Registry.view<TransformComponent, CameraComponent>().each([&](auto entity, auto& transformComponent, auto& cameraComponent) {
			if (cameraComponent.Primary) {
				mainCamera = &cameraComponent.Camera;
				cameraTransform = transformComponent.GetTransform();
			}
			});

		if (mainCamera) {
			Renderer2D::BeginScene(*mainCamera, cameraTransform);

			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group) {
				auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
				
				Entity e{ entity, this };
				glm::mat4 worldTransform = GetWorldTransform(e);

				Renderer2D::DrawSprite(worldTransform, sprite, (int)entity);
			}

			Renderer2D::EndScene();
		}
	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{
		Renderer2D::BeginScene(camera);

		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : group) {
			auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
			
			Entity e{ entity, this };
			glm::mat4 worldTransform = GetWorldTransform(e);

			Renderer2D::DrawSprite(worldTransform, sprite, (int)entity);
		}

		Renderer2D::EndScene();
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		m_ViewportHeight = height;
		m_ViewportWidth = width;

		//Resize our non-fixed aspect ratio cameras
		auto view = m_Registry.view<CameraComponent>();
		for (auto entity : view) {
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio) {
				cameraComponent.Camera.SetViewportSize(width, height);
			}
		}
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

		// Inicialitzem el viewport
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

		// Si té pare, multipliquem la nostra transformació per la del pare
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
}