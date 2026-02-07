#pragma once
#include "Runic2D.h"
#include "TimerScript.h"
#include "UpgradeCard.h"
#include "GameComponents.h" 

using namespace Runic2D;

enum class UIState {
    Gameplay,
    LevelUp
};

class HUDManager : public ScriptableEntity {
public:
    Entity m_TimerEntity;
    Entity m_HealthBarEntity;
    Entity m_XPBarEntity;
    Entity m_LevelTextEntity;

    std::vector<Entity> m_UpgradeCards;

    UIState m_CurrentState = UIState::Gameplay;

    void OnCreate() override {
        CreateGameplayHUD();

		auto& ctx = GameContext::Get();

        ctx.OnLevelUp = [this](int level) { ShowLevelUp(level); };

        ctx.OnHealthChanged = [this](float current, float max) {
            float percent = (max > 0) ? (current / max) : 0.0f;
            R2D_INFO("HUD: Health Updated to {0}%", percent * 100);
            if (m_HealthBarEntity) {
                auto& tc = m_HealthBarEntity.GetComponent<TransformComponent>();
                tc.Scale.x = percent * 2.0f;
				tc.IsDirty = true;
            }
        };

        ctx.OnXPChanged = [this](float current, float max) {
            float percent = (max > 0) ? (current / max) : 0.0f;
            if (m_XPBarEntity) {
                auto& tc = m_XPBarEntity.GetComponent<TransformComponent>();
                tc.Scale.x = percent * 5.0f;
                tc.IsDirty = true;
            }
        };

        ctx.OnHealthChanged(ctx.PlayerHealth, ctx.PlayerMaxHealth);
        ctx.OnXPChanged(ctx.CurrentXP, ctx.MaxXP);
        if (m_LevelTextEntity) {
            m_LevelTextEntity.GetComponent<TextComponent>().TextString = "LVL " + std::to_string(ctx.CurrentLevel);
        }
    }

    void OnUpdate(Timestep ts) override {
        if (m_CurrentState == UIState::LevelUp && GameContext::Get().State == GameState::Running) {
            SetState(UIState::Gameplay);
        }
    }

    void OnDestroy() override {
        GameContext::Get().OnLevelUp = nullptr;
    }

private:
    void SetState(UIState state) {
        m_CurrentState = state;
        switch (state) {
        case UIState::Gameplay:
            if (m_TimerEntity) m_TimerEntity.GetComponent<TextComponent>().Visible = true;
            ClearUpgradeMenu();
            break;

        case UIState::LevelUp:
            if (m_TimerEntity) m_TimerEntity.GetComponent<TextComponent>().Visible = false;
            break;
        }
    }

    void ClearUpgradeMenu() {
        for (auto card : m_UpgradeCards) {
            if (card) {
                GetScene()->DestroyEntity(card);
            }
        }
        m_UpgradeCards.clear();
    }

    void CreateGameplayHUD() {
		// Timer
        m_TimerEntity = GetScene()->CreateEntity("TimerText");
        auto& tc = m_TimerEntity.GetComponent<TransformComponent>();
        tc.SetTranslation({ -0.65f, 4.5f, 0.1f });
        tc.SetScale({ 0.75f, 0.75f, 1.0f });

        auto& txt = m_TimerEntity.AddComponent<TextComponent>();
        txt.TextString = "00:00";

        m_TimerEntity.AddComponent<NativeScriptComponent>().Bind<TimerScript>();

		// Health Bar
        m_HealthBarEntity = GetScene()->CreateEntity("HealthBar");
        auto& hbTc = m_HealthBarEntity.GetComponent<TransformComponent>();
        hbTc.SetTranslation({ -4.0f, 4.0f, 0.1f });
        hbTc.SetScale({ 2.0f, 0.3f, 1.0f });
        auto& hbSprite = m_HealthBarEntity.AddComponent<SpriteRendererComponent>();
		hbSprite.Color = { 1.0f, 0.2f, 0.2f, 1.0f };

		// XP Bar
        m_XPBarEntity = GetScene()->CreateEntity("XPBar");
        auto& xpTc = m_XPBarEntity.GetComponent<TransformComponent>();
        xpTc.SetTranslation({ 0.0f, -4.0f, 0.1f });
        xpTc.SetScale({ 0.0f, 0.2f, 1.0f }); 
		auto& xpSprite = m_XPBarEntity.AddComponent<SpriteRendererComponent>();
		xpSprite.Color = { 0.2f, 0.2f, 1.0f, 1.0f };

        // Level Text
        m_LevelTextEntity = GetScene()->CreateEntity("LevelText");
        auto& lvlTc = m_LevelTextEntity.GetComponent<TransformComponent>();
        lvlTc.SetTranslation({ 4.0f, 4.5f, 0.1f });
        lvlTc.SetScale({ 0.75f, 0.75f, 1.0f });
        auto& lvlTxt = m_LevelTextEntity.AddComponent<TextComponent>();
		lvlTxt.TextString = "LVL 1";
    }

    void ShowLevelUp(int level) {
        R2D_INFO("HUD Manager: Canviant a mode Level Up!");
        SetState(UIState::LevelUp);
        CreateUpgradeMenu();
        // Aquí també podries actualitzar el text de les cartes segons el nivell
        if (m_LevelTextEntity) {
            m_LevelTextEntity.GetComponent<TextComponent>().TextString = "LVL " + std::to_string(level);
        }
    }

    void CreateUpgradeMenu() {

        std::vector<UpgradeDef> cards = UpgradeDatabase::GetRandomUniqueUpgrades(3);

        float spacing = 3.5f;
        int count = (int)cards.size();
        float startX = -((count - 1) * spacing) / 2.0f;

        for (int i = 0; i < count; i++) {
            UpgradeDef data = cards[i];

            Entity card = GetScene()->CreateEntity("UpgradeCard");

            auto& tc = card.GetComponent<TransformComponent>();
            tc.SetTranslation({startX + (i * spacing), 0.0f, 0.5f});
            tc.SetScale({3.0f, 4.0f, 1.0f});

            auto& sprite = card.AddComponent<SpriteRendererComponent>();
            sprite.Color = { 1.0f, 1.0f, 1.0f, 1.0f };

            card.AddComponent<UpgradeComponent>(data);
            card.AddComponent<NativeScriptComponent>().Bind<UpgradeCard>();

            m_UpgradeCards.push_back(card);
        }
    }
};