#pragma once
#include "Runic2D.h"
#include "GameComponents.h"
#include "Player.h" 

using namespace Runic2D;

class ExperienceOrb : public ScriptableEntity {
public:
    bool m_IsMagnetized = false;
    Entity m_PlayerEntity;
    float m_CurrentSpeed = 0.0f;
    float m_Acceleration = 15.0f;

    void OnCreate() override {
        Entity player = GetScene()->GetEntityWithComponent<PlayerStatsComponent>();

        if (player) {
            glm::vec2 myPos = GetComponent<TransformComponent>().Translation;
            glm::vec2 playerPos = player.GetComponent<TransformComponent>().Translation;

            float pickupRadius = 1.0f;

            if (player.HasComponent<CircleCollider2DComponent>()) {
                auto& cc = player.GetComponent<CircleCollider2DComponent>();
                auto& tc = player.GetComponent<TransformComponent>();

                float maxScale = std::max(std::abs(tc.Scale.x), std::abs(tc.Scale.y));
                pickupRadius = cc.Radius * maxScale;

                playerPos += cc.Offset; 
            }

            glm::vec2 diff = playerPos - myPos;
            float distSq = glm::dot(diff, diff);
            float radiusSq = pickupRadius * pickupRadius;

            if (distSq < radiusSq) {
                m_IsMagnetized = true;
                m_PlayerEntity = player;
                m_CurrentSpeed = 2.0f;
            }
        }
    }

    void OnUpdate(Timestep ts) override {
        if (!m_IsMagnetized) {
            return;
        }

        if (m_PlayerEntity) {
            auto& myTrans = GetComponent<TransformComponent>();
            auto& playerTrans = m_PlayerEntity.GetComponent<TransformComponent>();

            glm::vec2 targetPos = playerTrans.Translation;
            glm::vec2 myPos = myTrans.Translation;

            glm::vec2 diff = targetPos - myPos;
            float distSq = glm::dot(diff, diff);

            if (distSq < 0.25f) {
                Collect();
                return;
            }

            glm::vec2 dir = glm::normalize(diff);

            m_CurrentSpeed += m_Acceleration * ts;

            auto& rb = GetComponent<Rigidbody2DComponent>();
            b2BodyId bodyId = rb.RuntimeBody;

            b2Body_SetLinearVelocity(bodyId, { dir.x * m_CurrentSpeed, dir.y * m_CurrentSpeed });
        }
    }

    void OnCollision(Entity other) override {
        if (m_IsMagnetized) return;

        if (other.HasComponent<PlayerStatsComponent>()) {
            m_IsMagnetized = true;
            m_PlayerEntity = other;
            m_CurrentSpeed = 5.0f;
        }
    }

private:
    void Collect() {
        int xpAmount = GetComponent<ExperienceComponent>().Amount;
        m_PlayerEntity.GetComponent<PlayerStatsComponent>().AddExperience(xpAmount);

        // So de "Ding!"

        Destroy();
    }
};