#pragma once
#include "Runic2D.h"
#include "GameContext.h"
#include "GameComponents.h"
#include "Runic2D/Utils/SceneUtils.h"

using namespace Runic2D;

class UpgradeCard : public ScriptableEntity {
public:
    UpgradeDef m_Data;
    bool m_IsPressed = false;

    void OnCreate() override {
        if (HasComponent<UpgradeComponent>()) {
            m_Data = GetComponent<UpgradeComponent>().Data;

            if (HasComponent<SpriteRendererComponent>()) {
                GetComponent<SpriteRendererComponent>().Color = m_Data.Color;
            }
            // Aquí també posaries el text del títol quan tinguis TextComponent
        }
    }

    void OnUpdate(Timestep ts) override {
        bool hover = IsMouseOver();

        if (hover && Input::IsMouseButtonPressed(MouseButton::Left)) {
            m_IsPressed = true;
            GetComponent<TransformComponent>().SetScale({ 2.8f, 3.8f, 1.0f });
        }

        if (m_IsPressed && !Input::IsMouseButtonPressed(MouseButton::Left)) {
            m_IsPressed = false;
            if (hover) {
                ApplyUpgrade();
            }
            else {
                GetComponent<TransformComponent>().SetScale({ 3.0f, 4.0f, 1.0f });
            }
        }
    }

private:
    void ApplyUpgrade() {
        R2D_INFO("Millora triada: {0}", m_Data.Title);

        // TODO: GameContext::Get().ApplyUpgrade(m_Data.Type);

        // Simplement diem que tornem a jugar.
        // El HUDManager detectarà el canvi i esborrarà les cartes.
        GameContext::Get().State = GameState::Running;
    }

    bool IsMouseOver() {
        glm::vec2 mouseWorld = Utils::SceneUtils::GetMouseWorldPosition(GetScene());
        if (!HasComponent<TransformComponent>()) return false;
        auto& tc = GetComponent<TransformComponent>();
        float halfW = 0.5f * tc.Scale.x;
        float halfH = 0.5f * tc.Scale.y;
        return (mouseWorld.x >= tc.Translation.x - halfW && mouseWorld.x <= tc.Translation.x + halfW &&
            mouseWorld.y >= tc.Translation.y - halfH && mouseWorld.y <= tc.Translation.y + halfH);
    }
};