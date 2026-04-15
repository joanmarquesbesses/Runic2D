#pragma once

#include "Runic2D.h"
#include "Entities/EntityFactory.h"
#include "Systems/UpgradeSystem.h"
#include "GameComponents.h"

using namespace Runic2D;

namespace Survivor
{
    class GameManager : public ScriptableEntity {
    public:
        void OnCreate() override {

            auto& stats = GetEntity().AddComponent<GameStatsComponent>();
            stats.State = GameState::Running;
            stats.TimeAlive = 0.0f;

            stats.OnUpgradeApplied = [this](UpgradeType type)
                {
                    ApplyUpgradeToPlayer(type);
                };

            EntityFactory::Init(GetScene());
        }

        virtual void OnUpdate(Timestep ts) override;

        void OnDestroy() override {
            if (HasComponent<GameStatsComponent>()) {
                GetComponent<GameStatsComponent>().OnUpgradeApplied = nullptr;
            }

            EntityFactory::Shutdown();
        }

    private:
        void SpawnEnemy();
        glm::vec2 GetRandomOffScreenPosition();
        void ApplyUpgradeToPlayer(UpgradeType type);

    private:
        float m_SpawnTimer = 0.0f;
        float m_SpawnCooldown = 0.2f;
    };

}