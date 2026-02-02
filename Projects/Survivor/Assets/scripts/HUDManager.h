#pragma once
#include "Runic2D.h"
#include "GameContext.h"
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
    UIState m_CurrentState = UIState::Gameplay;

    std::vector<Entity> m_UpgradeCards;

    void OnCreate() override {
        CreateGameplayHUD();
        GameContext::Get().OnLevelUp = [this](int level) { ShowLevelUp(level); };
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
        m_TimerEntity = GetScene()->CreateEntity("TimerText");
        auto& tc = m_TimerEntity.GetComponent<TransformComponent>();
        tc.SetTranslation({ -0.65f, 4.5f, 0.1f });
        tc.SetScale({ 0.75f, 0.75f, 1.0f });

        auto& txt = m_TimerEntity.AddComponent<TextComponent>();
        txt.TextString = "00:00";

        m_TimerEntity.AddComponent<NativeScriptComponent>().Bind<TimerScript>();
    }

    void ShowLevelUp(int level) {
        R2D_INFO("HUD Manager: Canviant a mode Level Up!");
        SetState(UIState::LevelUp);
        CreateUpgradeMenu();
        // Aquí també podries actualitzar el text de les cartes segons el nivell
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