#pragma once
#include "Runic2D.h"
#include "Core/GameComponents.h"
#include "Player.h" 

using namespace Runic2D;

namespace Survivor {

    class ExperienceOrb : public ScriptableEntity {
    public:
        Entity m_PlayerEntity;
		Entity m_GameManagerEntity;
        float m_CurrentSpeed = 0.0f;
        float m_Acceleration = 15.0f;

        void OnCreate() override {
            m_PlayerEntity = GetScene()->GetEntityWithComponent<PlayerStatsComponent>();
			m_GameManagerEntity = GetScene()->GetEntityWithComponent<GameStatsComponent>();

            if (m_PlayerEntity) {
                glm::vec2 myPos = GetComponent<TransformComponent>().GetTranslation();
                glm::vec2 playerPos = m_PlayerEntity.GetComponent<TransformComponent>().GetTranslation();

                float pickupRadius = 1.0f;

                if (m_PlayerEntity.HasComponent<CircleCollider2DComponent>()) {
                    auto& cc = m_PlayerEntity.GetComponent<CircleCollider2DComponent>();
                    auto& tc = m_PlayerEntity.GetComponent<TransformComponent>();

                    float maxScale = std::max(std::abs(tc.GetScale().x), std::abs(tc.GetScale().y));
                    pickupRadius = cc.Radius * maxScale;

                    playerPos += cc.Offset;
                }

                glm::vec2 diff = playerPos - myPos;
                float distSq = glm::dot(diff, diff);
                float radiusSq = pickupRadius * pickupRadius;

                if (distSq < radiusSq) {
                    Magnetize(m_PlayerEntity);
                }
            }
        }

        void OnUpdate(Timestep ts) override {
            if (!GetComponent<ExperienceComponent>().Magnetized) {
                return;
            }

            if (m_PlayerEntity) {
                auto& myTrans = GetComponent<TransformComponent>();
                auto& playerTrans = m_PlayerEntity.GetComponent<TransformComponent>();

                glm::vec2 diff = playerTrans.GetTranslation() - myTrans.GetTranslation();
                float distSq = glm::dot(diff, diff);

                if (distSq < 0.25f) {
                    Collect();
                    return;
                }

                glm::vec2 dir = glm::normalize(diff);

                m_CurrentSpeed += m_Acceleration * ts;

                if (HasComponent<Rigidbody2DComponent>()) {
                    auto& rb = GetComponent<Rigidbody2DComponent>();
                    b2Body_SetLinearVelocity(rb.RuntimeBody, { dir.x * m_CurrentSpeed, dir.y * m_CurrentSpeed });
                }
                else {
                    myTrans.SetTranslation(myTrans.GetTranslation() + glm::vec3(dir * m_CurrentSpeed * (float)ts, 0.0f));
                }
            }
        }

        void OnSensor(Entity other) override {
            if (GetComponent<ExperienceComponent>().Magnetized) return;

            if (other.HasComponent<PlayerStatsComponent>()) {
                Magnetize(other);
            }
        }

    private:
        void Magnetize(Entity player) {
            GetComponent<ExperienceComponent>().Magnetized = true;
            m_PlayerEntity = player;
            m_CurrentSpeed = 2.0f;
        }

        void Collect() {
            int xpAmount = 1;
            if (HasComponent<ExperienceComponent>())
                xpAmount = GetComponent<ExperienceComponent>().Amount;

			m_GameManagerEntity.GetComponent<GameStatsComponent>().AddXP(xpAmount);

            // So de "Ding!"

            Destroy();
        }
    };

}
