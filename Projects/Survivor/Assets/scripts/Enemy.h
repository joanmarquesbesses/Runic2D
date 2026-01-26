#pragma once

#include "Runic2D.h"

using namespace Runic2D;

class Enemy : public ScriptableEntity {
public:

    Entity m_Player;

    virtual void OnCreate() override {
        m_Player = GetScene()->FindEntityByName("Player");
    }

    virtual void OnUpdate(Timestep ts) override {
        auto& stats = GetComponent<EnemyStatsComponent>();

        if (stats.Health <= 0) {
            Die();
            return;
        }

        MoveTowardsPlayer(ts, stats);
    }


    virtual void MoveTowardsPlayer(float ts, EnemyStatsComponent& stats) {

        if (!m_Player)
        {
            m_Player = GetScene()->FindEntityByName("Player");
            if (!m_Player) return;
        }

        auto& transform = GetComponent<TransformComponent>();

        glm::vec2 playerPos = m_Player.GetComponent<TransformComponent>().Translation;
        glm::vec2 myPos = transform.Translation;

        glm::vec2 difference = playerPos - myPos;
        float distance = glm::length(difference);

        if (distance < 0.1f) return;

        glm::vec2 direction = glm::normalize(difference);

        auto& rb = GetComponent<Rigidbody2DComponent>();
		b2BodyId bodyId = rb.RuntimeBody;
		b2Body_SetLinearVelocity(bodyId, { direction.x * stats.Speed, direction.y * stats.Speed });

        if (direction.x < 0) transform.Scale.x = fabs(transform.Scale.x);
		else transform.Scale.x = -fabs(transform.Scale.x);
    }

    void TakeDamage(float amount) {
        if (HasComponent<EnemyStatsComponent>()) {
            auto& stats = GetComponent<EnemyStatsComponent>();
            stats.Health -= amount;
        }
        // Visual feedback (Flash blanc/vermell)
        // Spawn Floating Text number
    }

    void Die() {
        // Spawn XP Gem
        // Play death sound
        Destroy();
    }
};