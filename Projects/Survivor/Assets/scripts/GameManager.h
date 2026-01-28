#pragma once

#include "Runic2D.h"
#include "EntityFactory.h"

using namespace Runic2D;

class GameManager : public ScriptableEntity {
public:
    virtual void OnUpdate(Timestep ts) override;

private:
    void SpawnEnemy();
    glm::vec2 GetRandomOffScreenPosition();

private:
    float m_SpawnTimer = 0.0f;
    float m_SpawnCooldown = 0.2f;

    Entity m_TimerEntity;   
    float m_GameTime = 0.0f;
};