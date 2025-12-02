#include "R2Dpch.h"
#include "Scene.h"

#include "Component.h"
#include "Runic2D/Renderer/Renderer2D.h"

#include "Entity.h"

namespace Runic2D {

	static void DoMath(const glm::mat4& transform) {

	}

	Scene::Scene()
	{

	}

	Scene::~Scene()
	{
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		Entity entity = { m_Registry.create(), this };
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy((entt::entity)entity);
	}

	void Scene::OnUpdate(Timestep ts)
	{
		Camera* mainCamera = nullptr;
		glm::mat4* cameraTransform = nullptr;
		//Render Sprites
		m_Registry.view<TransformComponent, CameraComponent>().each([&](auto entity, auto& transformComponent, auto& cameraComponent) {
			if (cameraComponent.Primary) {
				mainCamera = &cameraComponent.Camera;
				cameraTransform = &transformComponent.Transform;
			}
			});

		if (mainCamera) {
			Renderer2D::BeginScene(mainCamera->GetProjection(), *cameraTransform);

			auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
			for (auto entity : group) {
				auto tuple = group.get<TransformComponent, SpriteRendererComponent>(entity);
				auto& transform = std::get<0>(tuple);
				auto& sprite = std::get<1>(tuple);

				Renderer2D::DrawQuad(transform.Transform, sprite.Color);
			}

			Renderer2D::EndScene();
		}
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

}