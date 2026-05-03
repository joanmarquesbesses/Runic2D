
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

        bool UpdateWhenPaused() override { return true; }

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
                GetScene()->SetPaused(true);
                  CreateUpgradeMenu();
            }
            else if (stats.State == GameState::Running && m_CurrentState == UIState::LevelUp)
            {
                m_CurrentState = UIState::Gameplay;
                if (m_TimerText) m_TimerText.GetComponent<TextComponent>().Visible = true;
                GetScene()->SetPaused(false);
                  ClearUpgradeMenu();
            }
        }

        void CreateUpgradeMenu()
        {
            auto cards = UpgradeDatabase::GetRandomUniqueUpgrades(3);

            const float cardWidth = 300.0f;
            const float cardHeight = 400.0f;
            const float spacing = 50.0f;
            int count = (int)cards.size();

            float totalWidth = (cardWidth * count) + (spacing * (count - 1));
            float startX = (1920.0f * 0.5f) - (totalWidth * 0.5f) + (cardWidth * 0.5f);

            for (int i = 0; i < count; i++)
            {
                Entity card = GetScene()->CreateEntity("UpgradeCard");

                auto& rect = card.AddComponent<RectTransformComponent>();
                rect.AnchorMin = { 0.5f, 0.5f };
                rect.AnchorMax = { 0.5f, 0.5f };
                rect.Pivot = { 0.5f, 0.5f };
                rect.Size = { cardWidth, cardHeight };

                float xOffset = (float)i * (cardWidth + spacing) - ((float)(count - 1) * (cardWidth + spacing) * 0.5f);
                rect.Position = { xOffset, 0.0f };
                rect.ZIndex = 10;

                auto& sprite = card.AddComponent<SpriteRendererComponent>();
                sprite.Texture = cards[i].CardTexture;

                auto& btn = card.AddComponent<ButtonComponent>();

                Entity titleEnt = GetScene()->CreateEntity("Card_Title");
                GetScene()->ParentEntity(titleEnt, card);

                auto& titleRect = titleEnt.AddComponent<RectTransformComponent>();
                titleRect.AnchorMin = { 0.5f, 1.0f };
                titleRect.AnchorMax = { 0.5f, 1.0f };
                titleRect.Pivot = { 0.5f, 1.0f };
                titleRect.Position = { 0.0f, -20.0f };
                titleRect.Size = { cardWidth * 0.9f, 40.0f };
                titleRect.ZIndex = 9;

                auto& titleTxt = titleEnt.AddComponent<TextComponent>();
                titleTxt.SetText(cards[i].Title);
                titleTxt.TextAlignment = TextComponent::Alignment::Center;
                titleTxt.Color = { 1.f, 1.f, 1.f, 1.f };


                Entity descEnt = GetScene()->CreateEntity("Card_Desc");
                GetScene()->ParentEntity(descEnt, card);

                auto& descRect = descEnt.AddComponent<RectTransformComponent>();
                descRect.AnchorMin = { 0.5f, 0.0f };
                descRect.AnchorMax = { 0.5f, 0.0f };
                descRect.Pivot = { 0.5f, 0.0f };
                descRect.Position = { 0.0f, 20.0f };
                descRect.Size = { cardWidth * 0.9f, 30.0f };
                descRect.ZIndex = 9;

                auto& descTxt = descEnt.AddComponent<TextComponent>();
                descTxt.SetText(cards[i].Description);
                descTxt.TextAlignment = TextComponent::Alignment::Center;
                descTxt.Color = { 0.85f, 0.85f, 0.85f, 1.f };

                auto& upgradeComp = card.AddComponent<UpgradeComponent>(cards[i]);
                upgradeComp.TitleEntity = titleEnt;
                upgradeComp.DescEntity = descEnt;

                card.AddComponent<NativeScriptComponent>().Bind<UpgradeCard>();

                m_UpgradeCards.push_back(card);
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