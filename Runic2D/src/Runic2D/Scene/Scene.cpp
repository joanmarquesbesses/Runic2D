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
		auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		for (auto entity : group) {
			auto tuple = group.get<TransformComponent, SpriteRendererComponent>(entity);
			auto& transform = std::get<0>(tuple);
			auto& sprite = std::get<1>(tuple);

			Renderer2D::DrawQuad(transform.Transform, sprite.Color);
		}
	}

}