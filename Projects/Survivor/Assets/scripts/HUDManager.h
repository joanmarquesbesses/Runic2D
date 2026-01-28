#pragma once
#include "Runic2D.h"
#include "GameContext.h"
#include "TimerScript.h"

using namespace Runic2D;

enum class UIState {
    Gameplay,
    LevelUp,
    GameOver
};

class HUDManager : public ScriptableEntity {
public:
    Entity m_TimerEntity;

    void OnCreate() override {
        CreateGameplayHUD();

        /*GameContext::Get().OnLevelUp = [this](int newLevel) {
            ShowLevelUp(newLevel);
            };*/

        SetState(UIState::Gameplay);
    }

    void OnUpdate(Timestep ts) override {
        // Aquí podries gestionar animacions globals de UI si cal
    }

private:
    void SetState(UIState state) {
        switch (state) {
        case UIState::Gameplay:
            if (m_TimerEntity) m_TimerEntity.GetComponent<TextComponent>().Visible = true;
            break;

        case UIState::LevelUp:
            if (m_TimerEntity) m_TimerEntity.GetComponent<TextComponent>().Visible = false;
            break;
        }
    }

    void CreateGameplayHUD() {
        m_TimerEntity = GetScene()->CreateEntity("TimerText");
        auto& tc = m_TimerEntity.GetComponent<TransformComponent>();
        tc.Translation = { -0.65f, 4.5f, 0.1f };
        tc.Scale = { 0.75f, 0.75f, 1.0f };

        auto& txt = m_TimerEntity.AddComponent<TextComponent>();
        txt.TextString = "00:00";

        m_TimerEntity.AddComponent<NativeScriptComponent>().Bind<TimerScript>();
    }

    void ShowLevelUp(int level) {
        R2D_INFO("HUD Manager: Canviant a mode Level Up!");
        SetState(UIState::LevelUp);

        // Aquí també podries actualitzar el text de les cartes segons el nivell
    }
};