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
                m_BaseScale = GetComponent<RectTransformComponent>().GetScale();
            }

            if (HasComponent<ButtonComponent>()) {
                auto& btn = GetComponent<ButtonComponent>();
                btn.OnClick = [this]() { SelectUpgrade(); };

                btn.OnHover = [this]() {
                    Tween::Clear(GetEntity());
                    Tween::To(GetEntity(), TweenTarget::Scale, { m_BaseScale.x * 1.15f, m_BaseScale.y * 1.15f, 1.0f, 0.0f }, 0.2f, EaseType::EaseOutQuad);
                };

                btn.OnUnhover = [this]() {
                    Tween::Clear(GetEntity());
                    Tween::To(GetEntity(), TweenTarget::Scale, { m_BaseScale.x, m_BaseScale.y, 1.0f, 0.0f }, 0.2f, EaseType::EaseOutQuad);
                };
            }
        }

        bool UpdateWhenPaused() override { return true; }

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
