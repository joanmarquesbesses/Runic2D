#pragma once
#include "Runic2D.h"
#include "Core/GameComponents.h"
#include "Runic2D/Utils/SceneUtils.h"

using namespace Runic2D;

namespace Survivor {

    class UpgradeCard : public ScriptableEntity {
    public:
        UpgradeDef m_Data;
        bool m_IsPressed = false;

        glm::vec3 m_BaseScale = glm::vec3(1.f);
        glm::vec3 m_TitleBaseScale = glm::vec3(0.35f);
        glm::vec3 m_DescBaseScale = glm::vec3(0.28f);
        bool m_Clicked = false;

        void OnCreate() override {
            m_BaseScale = GetComponent<TransformComponent>().Scale;
        }

        void OnUpdate(Timestep ts) override {
            glm::vec2 mouse = GetMouseUIPosition();
            auto& tc = GetComponent<TransformComponent>();
            auto& uc = GetComponent<UpgradeComponent>();

            float halfW = std::abs(m_BaseScale.x) * 0.5f;
            float halfH = std::abs(m_BaseScale.y) * 0.5f;

            bool hovered = mouse.x > tc.Translation.x - halfW &&
                mouse.x < tc.Translation.x + halfW &&
                mouse.y > tc.Translation.y - halfH &&
                mouse.y < tc.Translation.y + halfH;

            float hoverMultiplier = hovered ? 1.06f : 1.0f;

            tc.Scale = glm::mix(tc.Scale, m_BaseScale * hoverMultiplier, 0.15f);
			tc.IsDirty = true;

            float titleBaseY = 1.4f;
            float descBaseY = -1.2f;

            if (uc.TitleEntity) {
                auto& titleTc = uc.TitleEntity.GetComponent<TransformComponent>();
                titleTc.Scale = glm::mix(titleTc.Scale, m_TitleBaseScale * hoverMultiplier, 0.15f);
                float targetY = tc.Translation.y + (titleBaseY * hoverMultiplier);
                titleTc.Translation.y = glm::mix(titleTc.Translation.y, targetY, 0.15f);
                titleTc.IsDirty = true;
            }

            if (uc.DescEntity) {
                auto& descTc = uc.DescEntity.GetComponent<TransformComponent>();
                descTc.Scale = glm::mix(descTc.Scale, m_DescBaseScale * hoverMultiplier, 0.15f);
                float targetY = tc.Translation.y + (descBaseY * hoverMultiplier);
                descTc.Translation.y = glm::mix(descTc.Translation.y, targetY, 0.15f);
				descTc.IsDirty = true;
            }

            if (hovered && Input::IsMouseButtonPressed(MouseButton::Left) && !m_Clicked)
            {
                m_Clicked = true;
                SelectUpgrade();
            }

            if (!Input::IsMouseButtonPressed(MouseButton::Left))
                m_Clicked = false;
        }

    private:
        void SelectUpgrade()
        {
            if (!HasComponent<UpgradeComponent>()) return;

            Entity statsEntity = GetScene()->GetEntityWithComponent<GameStatsComponent>();
            if (!statsEntity) return;

            auto& stats = statsEntity.GetComponent<GameStatsComponent>();
            if (stats.OnUpgradeApplied)
                stats.OnUpgradeApplied(GetComponent<UpgradeComponent>().Data.Type);
        }

        glm::vec2 GetMouseUIPosition()
        {
            Entity uiCam = {};

            auto view = GetScene()->GetAllEntitiesWith<CameraComponent, UIComponent>();

            view.each([&](auto entity, auto& cam, auto& ui) {
                if (!uiCam) { 
                    uiCam = { entity, GetScene() };
                }
                });

            if (!uiCam)
                return Utils::SceneUtils::GetMouseWorldPosition(GetScene());

            auto& window = Application::Get().GetWindow();
            glm::vec2 input = Input::GetMousePosition();
            float width = (float)window.GetWidth();
            float height = (float)window.GetHeight();

            auto& camera = uiCam.GetComponent<CameraComponent>().Camera;
            auto& camTrans = uiCam.GetComponent<TransformComponent>();

            float x = (2.f * input.x) / width - 1.f;
            float y = 1.f - (2.f * input.y) / height;

            glm::mat4 invViewProj = glm::inverse(
                camera.GetProjection() * glm::inverse(camTrans.GetTransform())
            );

            glm::vec4 worldPos = invViewProj * glm::vec4(x, y, 1.f, 1.f);
            return { worldPos.x, worldPos.y };
        }
    };

}