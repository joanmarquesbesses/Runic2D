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
                auto& src = GetComponent<SpriteRendererComponent>();

                if (m_Data.CardTexture) {
                    src.Texture = m_Data.CardTexture;
                }
                else {
                    src.Color = m_Data.Color;
                }
            }

            CreateTextChild("TitleText", m_Data.Title, { 0.0f, 0.7f, 0.1f }, 0.1f);
            CreateTextChild("DescText", m_Data.Description, { 0.0f, 0.05f, 0.1f }, 0.1f);
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
        GameContext::Get().TriggerUpgradeApplied(m_Data.Type);
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

    void CreateTextChild(std::string name, std::string content, glm::vec3 offset, float size) {

		// espai entre línies
        float lineSpacingMultiplier = 1.0f;

        // 2. AJUST VERTICAL DE LA FONT (Baseline): 
        float fontBaselineOffset = 0.4f;

        // 3. AJUST HORITZONTAL MANUAL (Nudge):
        float manualXCorrection = 0.02f;

        std::vector<std::string> lines;
        std::stringstream ss(content);
        std::string segment;
        while (std::getline(ss, segment, '\n')) {
            lines.push_back(segment);
        }

        int totalLines = (int)lines.size();
        float lineHeight = size * lineSpacingMultiplier;

        float totalBlockHeight = (totalLines - 1) * lineHeight;
        float startY = offset.y + (totalBlockHeight / 2.0f) - fontBaselineOffset;

        auto font = Font::GetDefault();
        float kerning = 0.05f;

        for (int i = 0; i < totalLines; i++) {

            Entity textEnt = GetScene()->CreateEntity(name + "_" + std::to_string(i));
            GetScene()->ParentEntity(textEnt, GetEntity());

            float rawTextWidth = font->GetStringWidth(lines[i], kerning);
            float visualTextWidth = rawTextWidth * size;

            float centeredX = offset.x - (visualTextWidth / 2.0f) + manualXCorrection;

            float currentY = startY - (i * lineHeight);

            auto& tc = textEnt.GetComponent<TransformComponent>();
            tc.SetTranslation({ centeredX, currentY, offset.z });
            tc.SetScale({ size, size, 1.0f });

            auto& txt = textEnt.AddComponent<TextComponent>();
            txt.TextString = lines[i];
            txt.Color = { 0.0f, 0.0f, 0.0f, 1.0f };
            txt.Kerning = kerning;
            txt.FontAsset = font;
        }
    }
};