#include "R2Dpch.h"
#include "UISystem.h"

#include "Runic2D/Scene/Components/CoreComponents.h"
#include "Runic2D/Scene/Components/UIComponents.h"
#include "Runic2D/Scene/Components/RenderComponents.h"

#include "Runic2D/Core/Input.h"
#include "Runic2D/Scene/Tween.h"

#include "Runic2D/Renderer/RenderCommand.h"
#include "Runic2D/Renderer/Renderer2D.h"

namespace Runic2D {

	glm::vec2 UISystem::GetMousePositionInUISpace(Scene* scene)
	{
		float W = static_cast<float>(scene->GetViewportWidth());
		float H = static_cast<float>(scene->GetViewportHeight());
		if (W <= 0 || H <= 0) return { -1.0f, -1.0f };

		glm::vec2 mouse = Input::GetMousePosition();
		mouse.x -= scene->GetViewportBoundsMin().x;
		mouse.y -= scene->GetViewportBoundsMin().y;

		float ndcX = (2.0f * mouse.x) / W - 1.0f;
		float ndcY = 1.0f - (2.0f * mouse.y) / H;

		float aspectRatio = W / H;
		float refHeight = 1080.0f;
		float refWidth = refHeight * aspectRatio;

		glm::mat4 projection = glm::ortho(0.0f, refWidth, 0.0f, refHeight, -1.0f, 1.0f);
		glm::vec4 worldPos = glm::inverse(projection) * glm::vec4(ndcX, ndcY, 0.0f, 1.0f);

		return { worldPos.x, worldPos.y };
	}

	void UISystem::OnUpdate(Timestep ts, Scene* scene)
	{
		glm::vec2 mouseUI = GetMousePositionInUISpace(scene);

		bool mouseDown = Input::IsMouseButtonPressed(MouseButton::Left);

		auto& registry = scene->GetEntityRegistry();

		auto view = registry.view<ButtonComponent, RectTransformComponent>();

		view.each([&](entt::entity e, ButtonComponent& btn, RectTransformComponent& rect)
			{
				glm::mat4 inverseMesh = glm::inverse(rect.ComputedMeshTransform);
				glm::vec4 localMouse = inverseMesh * glm::vec4(mouseUI.x, mouseUI.y, 0.0f, 1.0f);

				bool hovered = (localMouse.x >= -0.5f && localMouse.x <= 0.5f &&
					localMouse.y >= -0.5f && localMouse.y <= 0.5f);

				ButtonComponent::State prevState = btn.CurrentState;

				if (!hovered)
				{
					if (prevState != ButtonComponent::State::Normal && btn.OnUnhover)
						btn.OnUnhover();

					btn.CurrentState = ButtonComponent::State::Normal;
				}
				else if (mouseDown)
				{
					if (prevState == ButtonComponent::State::Normal && btn.OnHover)
						btn.OnHover();

					btn.CurrentState = ButtonComponent::State::Pressed;
				}
				else
				{
					if (prevState == ButtonComponent::State::Normal && btn.OnHover)
						btn.OnHover();

					if (prevState == ButtonComponent::State::Pressed && btn.OnClick)
						btn.OnClick();

					btn.CurrentState = ButtonComponent::State::Hovered;
				}

				if (btn.CurrentState != prevState)
				{
					glm::vec4 targetColor = btn.NormalColor;
					if (btn.CurrentState == ButtonComponent::State::Hovered) targetColor = btn.HoveredColor;
					if (btn.CurrentState == ButtonComponent::State::Pressed) targetColor = btn.PressedColor;

					if (registry.all_of<SpriteRendererComponent>(e))
					{
						Tween::ClearTarget({ e, scene }, TweenTarget::Color);
						Tween::To({ e, scene }, TweenTarget::Color, targetColor, 0.15f, EaseType::EaseOutQuad);
					}
				}
			});
	}

	void UISystem::OnRender(Scene* scene)
	{
		float W = static_cast<float>(scene->GetViewportWidth());
		float H = static_cast<float>(scene->GetViewportHeight());
		float aspectRatio = W / H;

		float refHeight = 1080.0f;
		float refWidth = refHeight * aspectRatio;

		auto& registry = scene->GetEntityRegistry();

		glm::vec2 virtualViewportSize = { refWidth, refHeight };
		glm::vec2 virtualViewportPivot = { 0.0f, 0.0f }; 

		glm::mat4 projection = glm::ortho(0.0f, refWidth, 0.0f, refHeight, -1.0f, 1.0f);
		glm::mat4 view = glm::mat4(1.0f);

		RenderCommand::ClearDepth();
		Renderer2D::BeginScene(projection * view);

		glm::mat4 globalTransform = glm::mat4(1.0f);

		struct UIRenderCommand {
			int ZIndex;
			std::function<void()> RenderCall;
		};
		std::vector<UIRenderCommand> renderCommands;

		auto processEntity = [&](Entity e, const glm::mat4& parentWorldTransform, const glm::vec2& parentSize, const glm::vec2& parentPivot, auto& processEntityRef) -> void
			{
				if (!e.HasComponent<RectTransformComponent>()) return;

				auto& rectTransform = e.GetComponent<RectTransformComponent>();
				glm::mat4 worldTransform, meshTransform;
				if (rectTransform.m_IsDirty) {
					rectTransform.CalculateTransforms(parentWorldTransform, parentSize, parentPivot, worldTransform, meshTransform);
					rectTransform.WorldTransform = worldTransform;
					rectTransform.ComputedMeshTransform = meshTransform;
					rectTransform.m_IsDirty = false;
				}
				else {
					worldTransform = rectTransform.WorldTransform;
					meshTransform = rectTransform.ComputedMeshTransform;
				}

				entt::entity entityID = e;

				if (e.HasComponent<SpriteRendererComponent>()) {
					renderCommands.push_back({ rectTransform.ZIndex, [&registry, meshTransform, entityID]() {
						auto& sprite = registry.get<SpriteRendererComponent>(entityID);
						Renderer2D::DrawSprite(meshTransform, sprite, (int)(uint32_t)entityID);
					} });
				}
				if (e.HasComponent<CircleRendererComponent>()) {
					renderCommands.push_back({ rectTransform.ZIndex, [&registry, meshTransform, entityID]() {
						auto& circle = registry.get<CircleRendererComponent>(entityID);
						Renderer2D::DrawCircle(meshTransform, circle.Color, circle.Thickness, circle.Fade, (int)(uint32_t)entityID);
					} });
				}
				if (e.HasComponent<TextComponent>()) {
					renderCommands.push_back({ rectTransform.ZIndex, [&registry, worldTransform, rectTransform, entityID]() {
						auto& text = registry.get<TextComponent>(entityID);

						float fontSize = rectTransform.Size.y;
						float textWidth = text.GetTextWidth();
						float actualTextWidth = textWidth * fontSize;

						float boxLeftX = -rectTransform.Size.x * rectTransform.Pivot.x;

						float offsetX = boxLeftX;

						if (text.TextAlignment == TextComponent::Alignment::Center) {
							offsetX += (rectTransform.Size.x - actualTextWidth) * 0.5f;
						}
						else if (text.TextAlignment == TextComponent::Alignment::Right) {
							offsetX += (rectTransform.Size.x - actualTextWidth);
						}

						int numLines = 1;
						for (char c : text.GetText()) {
							if (c == '\n') numLines++;
						}

						float boxBottomY = -rectTransform.Size.y * rectTransform.Pivot.y;
						float extraHeight = (numLines - 1) * (fontSize + (text.LineSpacing * fontSize));
						float verticalShift = extraHeight * (1.0f - rectTransform.Pivot.y);
						float offsetY = boxBottomY + (fontSize * 0.25f) + verticalShift;

						glm::mat4 finalTextTransform = glm::translate(worldTransform, glm::vec3(offsetX, offsetY, 0.0f));
						finalTextTransform = glm::scale(finalTextTransform, glm::vec3(fontSize, fontSize, 1.0f));

						Renderer2D::DrawString(text.GetText(), text.FontAsset, finalTextTransform, text.Color, text.Kerning, text.LineSpacing, (int)(uint32_t)entityID, (int)text.TextAlignment);
					} });
				}

				if (e.HasComponent<RelationshipComponent>()) {
					auto& relationship = e.GetComponent<RelationshipComponent>();
					entt::entity currentChild = relationship.FirstChild;
					while (currentChild != entt::null) {
						Entity childEntity{ currentChild, scene };
						processEntityRef(childEntity, worldTransform, rectTransform.Size, rectTransform.Pivot, processEntityRef);
						currentChild = childEntity.GetComponent<RelationshipComponent>().NextSibling;
					}
				}
			};

		registry.view<RectTransformComponent>().each([&](auto entityID, auto& rect)
			{
				Entity e{ entityID, scene };
				bool isRoot = true;
				if (e.HasComponent<RelationshipComponent>()) {
					auto parent = e.GetComponent<RelationshipComponent>().Parent;
					if (parent != entt::null) {
						Entity parentEntity{ parent, scene };
						if (parentEntity.HasComponent<RectTransformComponent>()) {
							isRoot = false;
						}
					}
				}

				if (isRoot) {
					processEntity(e, globalTransform, virtualViewportSize, virtualViewportPivot, processEntity);
				}
			});

		std::sort(renderCommands.begin(), renderCommands.end(), [](const UIRenderCommand& a, const UIRenderCommand& b) {
			return a.ZIndex < b.ZIndex;
			});

		int lastZIndex = INT_MIN;
		for (const auto& cmd : renderCommands) {
			if (cmd.ZIndex != lastZIndex && lastZIndex != INT_MIN) {
				Renderer2D::NextBatch(Renderer2D::FlushReason::UIZIndexChange);
			}
			cmd.RenderCall();
			lastZIndex = cmd.ZIndex;
		}

		Renderer2D::EndScene();
	}
}
