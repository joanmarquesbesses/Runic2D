#pragma once
#include "Runic2D.h"
#include "Core/GameComponents.h"
#include "Runic2D/Utils/SceneUtils.h"

using namespace Runic2D;

namespace Survivor {

    class UpgradeCard : public ScriptableEntity {
    public:
        glm::vec2 m_BaseScale = glm::vec2(1.0f, 1.0f);

        void OnCreate() override {
            if (HasComponent<RectTransformComponent>()) {
                m_BaseScale = GetComponent<RectTransformComponent>().Scale;
            }

            if (HasComponent<ButtonComponent>()) {
                auto& btn = GetComponent<ButtonComponent>();
                btn.OnClick = [this]() {
                    SelectUpgrade();
                };
            }
        }

        void OnUpdate(Timestep ts) override {
            if (!HasComponent<ButtonComponent>() || !HasComponent<RectTransformComponent>())
                return;

            auto& btn = GetComponent<ButtonComponent>();
            auto& rect = GetComponent<RectTransformComponent>();
      
            float hoverMultiplier = (btn.CurrentState != ButtonComponent::State::Normal) ? 1.15f : 1.0f;

            glm::vec2 targetScale = m_BaseScale * hoverMultiplier;
            rect.Scale = glm::mix(rect.Scale, targetScale, 0.15f);
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
    };

}