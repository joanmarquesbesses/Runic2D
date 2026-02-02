#pragma once

#include "Runic2D.h"
#include "GameContext.h"
#include "EntityFactory.h"

using namespace Runic2D;

class GameManager : public ScriptableEntity {
public:
    // Configuració Nivell
    int m_Level = 1;
    float m_CurrentXP = 0.0f;
    float m_MaxXP = 100.0f;

    void OnCreate() override {
        auto& ctx = GameContext::Get();
        ctx.CurrentLevel = m_Level;
        ctx.CurrentXP = m_CurrentXP;
        ctx.MaxXP = m_MaxXP;
        ctx.State = GameState::Running;

        ctx.OnUpgradeApplied = [this](UpgradeType type) {
            ApplyUpgradeToPlayer(type);
            };
    }

    virtual void OnUpdate(Timestep ts) override;

    void OnDestroy() override {
        GameContext::Get().OnUpgradeApplied = nullptr;
    }

private:
    void LevelUP();
    void SpawnEnemy();
    glm::vec2 GetRandomOffScreenPosition();
    void ApplyUpgradeToPlayer(UpgradeType type);

private:
    float m_SpawnTimer = 0.0f;
    float m_SpawnCooldown = 0.2f;  
};