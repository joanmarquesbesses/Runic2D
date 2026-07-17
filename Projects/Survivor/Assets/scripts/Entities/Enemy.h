#pragma once

#include "Runic2D.h"
#include "Core/GameComponents.h"
#include "EntityFactory.h"

using namespace Runic2D;

namespace Survivor {

    class Enemy : public ScriptableEntity {
    public:

        Entity m_Player;
        float m_TintTime = 0.0f;
        float m_KnockbackTime = 0.0f;
        PhysicsSystem* m_PhysicsSystem = nullptr;

        bool m_IsDead = false;

        virtual void OnCreate() override {
            m_Player = GetScene()->GetEntityWithComponent<PlayerStatsComponent>();
            m_PhysicsSystem = GetScene()->GetSystem<PhysicsSystem>().get();
        }

        virtual void OnUpdate(Timestep ts) override {

            if (m_IsDead) {
                auto& anim = GetComponent<AnimationComponent>();
                if (anim.IsFinished()) {
                    Destroy();
                }
                return;
            }

            auto& stats = GetComponent<EnemyStatsComponent>();

            if (stats.Health <= 0) {
                Die();
                return;
            }

            if (m_TintTime > 0.0f)
            {
                m_TintTime -= ts;
                auto& sprite = GetComponent<SpriteRendererComponent>();
                sprite.Color = glm::vec4(1.0f, 0.35f, 0.35f, 1.0f);
                if (m_TintTime <= 0.0f)
                {
                    sprite.Color = glm::vec4(1.0f);
                    m_TintTime = 0.0f;
                }
            }

            if (m_KnockbackTime > 0.0f) {
                m_KnockbackTime -= ts;
                if (m_KnockbackTime <= 0.0f) {
                    auto& rb = GetComponent<Rigidbody2DComponent>();
                    if (B2_IS_NON_NULL(rb.RuntimeBody)) {
                        b2Body_SetLinearVelocity(rb.RuntimeBody, { 0.0f, 0.0f });

                        int shapeCount = b2Body_GetShapeCount(rb.RuntimeBody);
                        if (shapeCount > 0) {
                            std::vector<b2ShapeId> shapes(shapeCount);
                            b2Body_GetShapes(rb.RuntimeBody, shapes.data(), shapeCount);

                            for (auto shapeId : shapes) {
                                b2Filter filter = b2Shape_GetFilter(shapeId);
                                filter.maskBits |= PhysicsLayers::Enemy;

                                b2Shape_SetFilter(shapeId, filter);
                            }
                        }
                    }
                }
            }
            else {
                MoveTowardsPlayer(ts, stats);
            }
        }


        virtual void MoveTowardsPlayer(float ts, EnemyStatsComponent& stats) {

            if (!m_Player)
            {
                m_Player = GetScene()->GetEntityWithComponent<PlayerStatsComponent>();
                if (!m_Player) return;
            }

            auto& transform = GetComponent<TransformComponent>();

            glm::vec2 playerPos = m_Player.GetComponent<TransformComponent>().GetTranslation();
            glm::vec2 myPos = transform.GetTranslation();

            glm::vec2 difference = playerPos - myPos;
            float distance = glm::length(difference);

            if (distance < 0.1f) return;

            glm::vec2 direction = glm::normalize(difference);

            auto& rb = GetComponent<Rigidbody2DComponent>();
            b2BodyId bodyId = rb.RuntimeBody;
            b2Body_SetLinearVelocity(bodyId, { direction.x * stats.Speed, direction.y * stats.Speed });

            if (direction.x < 0) transform.SetScale({ fabs(transform.GetScale().x), transform.GetScale().y, transform.GetScale().z });
            else transform.SetScale({ -fabs(transform.GetScale().x), transform.GetScale().y, transform.GetScale().z });
        }

        void TakeDamage(float amount, glm::vec2 sourcePos) {
            if (m_IsDead) return;

            if (HasComponent<EnemyStatsComponent>()) {
                auto& stats = GetComponent<EnemyStatsComponent>();
                stats.Health -= amount;
            }

            m_TintTime = 0.15f;
            m_KnockbackTime = 0.2f;

            glm::vec2 myPos = GetComponent<TransformComponent>().GetTranslation();
            glm::vec2 knockbackDir = glm::normalize(myPos - sourcePos);

            auto& rb = GetComponent<Rigidbody2DComponent>();
            if (B2_IS_NON_NULL(rb.RuntimeBody)) {
                float knockbackForce = 1.5f;
                b2Body_ApplyLinearImpulse(rb.RuntimeBody, { knockbackDir.x * knockbackForce, knockbackDir.y * knockbackForce }, { 0.0,0.0 }, true);

                int shapeCount = b2Body_GetShapeCount(rb.RuntimeBody);
                if (shapeCount > 0) {
                    std::vector<b2ShapeId> shapes(shapeCount);
                    b2Body_GetShapes(rb.RuntimeBody, shapes.data(), shapeCount);

                    for (auto shapeId : shapes) {
                        b2Filter filter = b2Shape_GetFilter(shapeId);
                        filter.maskBits &= ~PhysicsLayers::Enemy;

                        b2Shape_SetFilter(shapeId, filter);
                    }
                }
            }

            bool isCrit = false;
            EntityFactory::CreateDamageText(myPos, amount, isCrit);
        }

        void Die() {
            if (m_IsDead) return;
            m_IsDead = true;

            int xpToDrop = 10;
            if (HasComponent<EnemyStatsComponent>()) {
                xpToDrop = GetComponent<EnemyStatsComponent>().XPDrop;
            }
            auto& transform = GetComponent<TransformComponent>();
            EntityFactory::CreateExperienceGem(transform.GetTranslation(), xpToDrop);

            if (HasComponent<CircleCollider2DComponent>()) {
                auto& coll = GetComponent<CircleCollider2DComponent>();
                coll.CategoryBits = 0;
                coll.MaskBits = 0;
                m_PhysicsSystem->UpdateEntityColliders(GetEntity());
            }

            if (HasComponent<Rigidbody2DComponent>()) {
                auto& rb = GetComponent<Rigidbody2DComponent>();
                if (B2_IS_NON_NULL(rb.RuntimeBody)) {
                    b2Body_SetLinearVelocity(rb.RuntimeBody, { 0.0f, 0.0f });
                }
            }

            if (HasComponent<AnimationComponent>()) {
                auto& ac = GetComponent<AnimationComponent>();
                ac.Play("Death");
            }
        }
    };

}
