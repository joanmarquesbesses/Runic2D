#include "R2Dpch.h"
#include "Scene.h"

#include "Component.h"
#include "Runic2D/Renderer/Renderer2D.h"

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
		m_Registry.destroy(entity);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };

		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		return entity;
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
				Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color);;
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
			Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color);;
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

}