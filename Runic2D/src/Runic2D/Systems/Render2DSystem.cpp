#include "R2Dpch.h"
#include "Render2DSystem.h"

#include "Runic2D/Scene/Component.h"
#include "Runic2D/Renderer/Renderer2D.h"

namespace Runic2D {

	void Render2DSystem::OnRender(Scene* scene)
	{
		auto cameraEntity = scene->GetPrimaryCameraEntity();
		if (cameraEntity) {

			auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
			auto& camTransform = cameraEntity.GetComponent<TransformComponent>();

			Renderer2D::BeginScene(camera, camTransform.GetTransform());

			auto& registry = scene->GetEntityRegistry();

			auto view = registry.view<TransformComponent, SpriteRendererComponent>(entt::exclude<RectTransformComponent>);

			view.each([&](auto entityID, auto& transform, auto& sprite)
				{
					Entity e{ entityID, scene };
					glm::mat4 worldTransform = e.GetWorldTransform();
					Renderer2D::DrawSprite(worldTransform, sprite, (int)entityID);
				});

			auto circleView = registry.view<TransformComponent, CircleRendererComponent>(entt::exclude<RectTransformComponent>);

			circleView.each([&](auto entityID, auto& transform, auto& circle)
				{
					Entity e{ entityID, scene };
					glm::mat4 worldTransform = e.GetWorldTransform();
					Renderer2D::DrawCircle(worldTransform, circle.Color, circle.Thickness, circle.Fade, (int)entityID);
				});

			registry.view<TransformComponent, TextComponent>(entt::exclude<RectTransformComponent>).each([&](auto entityID, auto& transform, auto& text)
				{
					if (!text.Visible)
						return;
					Entity e{ entityID, scene };
					glm::mat4 worldTransform = e.GetWorldTransform();
					Renderer2D::DrawString(text.GetText(), text.FontAsset, worldTransform, text.Color, text.Kerning, text.LineSpacing, (int)entityID, (int)text.TextAlignment);
				});

			Renderer2D::EndScene();
		}
	}
}
