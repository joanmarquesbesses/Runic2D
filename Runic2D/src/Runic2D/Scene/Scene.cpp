#include "R2Dpch.h"
#include "Scene.h"

#include "Component.h"
#include "Runic2D/Renderer/Renderer2D.h"

namespace Runic2D {

	static void DoMath(const glm::mat4& transform) {

	}

	Scene::Scene()
	{

	}

	Scene::~Scene()
	{
	}

	entt::entity Scene::CreateEntity()
	{
		return m_Registry.create();
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