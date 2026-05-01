
#pragma once
#include "Runic2D.h"
#include "Core/GameComponents.h"
#include "UI/UpgradeCard.h"        
#include "Systems/UpgradeDatabase.h" 
#include <sstream>
#include <iomanip>

using namespace Runic2D;

namespace Survivor {

    class HUDManager : public ScriptableEntity {
    public:
        void OnCreate() override {   
            // --- Timer (centre superior) ---
            m_TimerText = CreateUIText("HUD_Timer", "0:00");
            m_TimerText.GetComponent<TextComponent>().Color = { 0.27f, 0.27f, 0.27f, 1.0f };
            m_TimerText.GetComponent<RectTransformComponent>().AnchorMin = { 0.5f, 1.0f };
            m_TimerText.GetComponent<RectTransformComponent>().Pivot = { 0.5f, 1.0f };

            // --- Health (esquerra inferior) ---
            m_HealthText = CreateUIText("HUD_Health", "HP: 100");
            m_HealthText.GetComponent<TextComponent>().Color = { 0.2f, 1.0f, 0.2f, 1.0f };
            m_HealthText.GetComponent<TextComponent>().TextAlignment = TextComponent::Alignment::Left;
            m_HealthText.GetComponent<RectTransformComponent>().AnchorMin = { 0.0f, 0.0f };
            m_HealthText.GetComponent<RectTransformComponent>().Pivot = { 0.0f, 0.0f };
            m_HealthText.GetComponent<RectTransformComponent>().Position = { 0.0f, 75.0f };

            // --- XP / Nivell (dreta inferior) ---
            m_XPText = CreateUIText("HUD_XP", "Lv.1");
            m_XPText.GetComponent<TextComponent>().Color = { 0.4f, 0.8f, 1.0f, 1.0f };
            m_XPText.GetComponent<TextComponent>().TextAlignment = TextComponent::Alignment::Left;
            m_XPText.GetComponent<RectTransformComponent>().AnchorMin = { 0.0f, 0.0f };
            m_XPText.GetComponent<RectTransformComponent>().Pivot = { 0.0f, 0.0f };

            // --- FPS (cantonada superior dreta, debug) ---
            m_FPSText = CreateUIText("HUD_FPS", "FPS: 000");
            m_FPSText.GetComponent<TextComponent>().Color = { 0.5f, 0.5f, 0.5f, 1.0f };
			m_FPSText.GetComponent<TextComponent>().TextAlignment = TextComponent::Alignment::Left;
			m_FPSText.GetComponent<RectTransformComponent>().AnchorMin = { 0.0f, 1.0f };
			m_FPSText.GetComponent<RectTransformComponent>().Pivot = { 0.0f, 1.0f };
        }

        void OnDestroy() override {
            // Les entitats de UI es destruiran quan l'escena pari
            // No cal fer res manualment — l'escena gestiona el cicle de vida
        }

        void OnUpdate(Timestep ts) override {
            UpdateTimer();
            UpdateHealth();
            UpdateXP();
            UpdateFPS();
            UpdateLevelUpState();
        }

    private:
        // --- Helpers de creació ---
        Entity CreateUIText(const std::string& name,
            const std::string& initial)
        {
            Entity e = GetScene()->CreateEntity(name);
            auto& rec = e.AddComponent<RectTransformComponent>();

            auto& txt = e.AddComponent<TextComponent>();
			txt.SetText(initial);

            return e;
        }

        // --- Actualitzacions ---
        void UpdateTimer() {
            if (!m_TimerText) return;

            Entity statsEntity = GetScene()->GetEntityWithComponent<GameStatsComponent>();
            if (!statsEntity) return;

            float time = statsEntity.GetComponent<GameStatsComponent>().TimeAlive;

            int minutes = (int)(time / 60.0f);
            int seconds = (int)(time) % 60;

			if (seconds == m_LastSeconds) return;

            std::stringstream ss;
            ss << minutes << ":" << std::setfill('0') << std::setw(2) << seconds;

            auto& txt = m_TimerText.GetComponent<TextComponent>();
            txt.SetText(ss.str());
        }

        void UpdateHealth() {
            if (!m_HealthText) return;

            Entity playerEntity = GetScene()->GetEntityWithComponent<PlayerStatsComponent>();
            if (!playerEntity) return;

            auto& stats = playerEntity.GetComponent<PlayerStatsComponent>();
            int hp = (int)stats.Health;
            int maxHp = (int)stats.MaxHealth;

            m_HealthText.GetComponent<TextComponent>().SetText(
                "HP: " + std::to_string(hp) + "/" + std::to_string(maxHp));

            float ratio = stats.Health / stats.MaxHealth;
            m_HealthText.GetComponent<TextComponent>().Color = {
                1.0f - ratio, ratio, 0.0f, 1.0f
            };
        }

        void UpdateXP() {
            if (!m_XPText) return;

            Entity statsEntity = GetScene()->GetEntityWithComponent<GameStatsComponent>();
            if (!statsEntity) return;

            auto& stats = statsEntity.GetComponent<GameStatsComponent>();
            m_XPText.GetComponent<TextComponent>().SetText(
                "Lv." + std::to_string(stats.CurrentLevel) +
                "  XP: " + std::to_string((int)stats.CurrentXP) +
                "/" + std::to_string((int)stats.MaxXP));
        }

        void UpdateFPS() {
            if (!m_FPSText) return;
            m_FPSText.GetComponent<TextComponent>().SetText(
                "FPS: " + std::to_string((int)Application::Get().GetAverageFPS()));
        }

        void UpdateLevelUpState()
        {
            Entity statsEntity = GetScene()->GetEntityWithComponent<GameStatsComponent>();
            if (!statsEntity) return;
            auto& stats = statsEntity.GetComponent<GameStatsComponent>();

            if (stats.State == GameState::LevelUp && m_CurrentState != UIState::LevelUp)
            {
                m_CurrentState = UIState::LevelUp;
                if (m_TimerText) m_TimerText.GetComponent<TextComponent>().Visible = false;
                CreateUpgradeMenu();
            }
            else if (stats.State == GameState::Running && m_CurrentState == UIState::LevelUp)
            {
                m_CurrentState = UIState::Gameplay;
                if (m_TimerText) m_TimerText.GetComponent<TextComponent>().Visible = true;
                ClearUpgradeMenu();
            }
        }

        void CreateUpgradeMenu()
        {
            auto cards = UpgradeDatabase::GetRandomUniqueUpgrades(3);

            const float spacing = 3.5f;
            int count = (int)cards.size();
            float startX = -((count - 1) * spacing) / 2.f;

            for (int i = 0; i < count; i++)
            {
                Entity card = GetScene()->CreateEntity("UpgradeCard");
                card.AddComponent<RectTransformComponent>();

                auto& tc = card.GetComponent<TransformComponent>();
                tc.SetTranslation({ startX + i * spacing, 0.f, 0.1f }); // Z = 0.1
                tc.SetScale({ 3.f, 4.f, 1.f });

                auto& sprite = card.AddComponent<SpriteRendererComponent>();
                sprite.Texture = cards[i].CardTexture;
                sprite.Color = glm::vec4(1.f);

                Entity titleEnt = GetScene()->CreateEntity("Card_Title");
                titleEnt.AddComponent<RectTransformComponent>();
                auto& titleTxt = titleEnt.AddComponent<TextComponent>(cards[i].Title);
                titleTxt.Color = { 1.f, 1.f, 1.f, 1.f };

                float titleScale = 0.35f;
                float titleWidth = titleTxt.FontAsset->GetStringWidth(cards[i].Title, titleTxt.Kerning) * titleScale;

                titleEnt.GetComponent<TransformComponent>().SetTranslation({ (startX + i * spacing) - (titleWidth / 2.0f), 1.4f, 0.2f });
                titleEnt.GetComponent<TransformComponent>().SetScale({ titleScale, titleScale, 1.f });

                Entity descEnt = GetScene()->CreateEntity("Card_Desc");
                descEnt.AddComponent<RectTransformComponent>();
                auto& descTxt = descEnt.AddComponent<TextComponent>(cards[i].Description);
                descTxt.Color = { 0.85f, 0.85f, 0.85f, 1.f };

                float descScale = 0.28f;
                float descWidth = descTxt.FontAsset->GetStringWidth(cards[i].Description, descTxt.Kerning) * descScale;

                descEnt.GetComponent<TransformComponent>().SetTranslation({ (startX + i * spacing) - (descWidth / 2.0f), -1.2f, 0.2f });
                descEnt.GetComponent<TransformComponent>().SetScale({ descScale, descScale, 1.f });

                auto& upgradeComp = card.AddComponent<UpgradeComponent>(cards[i]);
                upgradeComp.TitleEntity = titleEnt; // Li passem les referències!
                upgradeComp.DescEntity = descEnt;   // Li passem les referències!

                card.AddComponent<NativeScriptComponent>().Bind<UpgradeCard>();

                m_UpgradeCards.push_back(card);
                m_UpgradeCards.push_back(titleEnt);
                m_UpgradeCards.push_back(descEnt);
            }
        }

        void ClearUpgradeMenu()
        {
            for (auto& card : m_UpgradeCards)
                GetScene()->DestroyEntity(card);
            m_UpgradeCards.clear();
        }

    private:
        float m_OrthoSize = 10.0f;
		int m_LastSeconds = -1;

        enum class UIState { Gameplay, LevelUp };
        UIState m_CurrentState = UIState::Gameplay;
        std::vector<Entity> m_UpgradeCards;

        Entity m_UICamera;
        Entity m_TimerText;
        Entity m_HealthText;
        Entity m_XPText;
        Entity m_FPSText;
    };
}