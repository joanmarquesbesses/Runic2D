#pragma once

#include "Runic2D.h"
#include "GameContext.h"
#include "EntityFactory.h"

using namespace Runic2D;

class GameManager : public ScriptableEntity {
public:
    void OnCreate() override {
        auto& ctx = GameContext::Get();
        ctx.CurrentLevel = 1;
        ctx.CurrentXP = 0.0f;
        ctx.MaxXP = 100.0f;
        ctx.State = GameState::Running;
		ctx.TimeAlive = 0.0f;

        ctx.OnUpgradeApplied = [this](UpgradeType type) {
            ApplyUpgradeToPlayer(type);
            };
    }

    virtual void OnUpdate(Timestep ts) override;

    void OnDestroy() override {
        GameContext::Get().OnUpgradeApplied = nullptr;
    }

private:
    void SpawnEnemy();
    glm::vec2 GetRandomOffScreenPosition();
    void ApplyUpgradeToPlayer(UpgradeType type);

private:
    float m_SpawnTimer = 0.0f;
    float m_SpawnCooldown = 0.2f;  
};