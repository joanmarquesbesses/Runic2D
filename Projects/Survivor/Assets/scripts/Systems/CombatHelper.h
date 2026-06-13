#pragma once

#include "Core/GameComponents.h"
#include "Entities/EntityFactory.h" 

using namespace Runic2D;

namespace Survivor {

    class CombatHelper {
    public:
        static void ApplyDamage(Runic2D::Entity target, float amount, glm::vec2 sourcePos)
        {
            if (target.HasComponent<DeadTag>()) return;

            if (target.HasComponent<EnemyStatsComponent>()) {
                auto& stats = target.GetComponent<EnemyStatsComponent>();
                stats.Health -= amount;

                if (stats.Health <= 0) {
                    target.AddComponent<DeadTag>();

                    EntityFactory::CreateExperienceGem(target.GetComponent<TransformComponent>().GetTranslation(), stats.XPDrop);

                    // Parem el cos físic
                    if (target.HasComponent<Runic2D::Rigidbody2DComponent>()) {
                        auto& rb = target.GetComponent<Runic2D::Rigidbody2DComponent>();
                        if (B2_IS_NON_NULL(rb.RuntimeBody)) b2Body_SetLinearVelocity(rb.RuntimeBody, { 0.0f, 0.0f });
                    }

                    // Tallem col·lisions
                    if (target.HasComponent<Runic2D::CircleCollider2DComponent>()) {
                        auto& coll = target.GetComponent<Runic2D::CircleCollider2DComponent>();
                        coll.CategoryBits = 0;
                        coll.MaskBits = 0;
                        target.GetScene()->GetSystem<Runic2D::PhysicsSystem>()->UpdateEntityColliders(target);
                    }

                    if (target.HasComponent<AnimationComponent>()) {
                        auto& ac = target.GetComponent<AnimationComponent>();
                        ac.Play("Death");
                    }
                    return;
                }
            }

            // 3. Si no ha mort, apliquem els Estats!

            // Afegim (o reiniciem) el Flash
            auto& flash = target.AddOrReplaceComponent<DamageFlashComponent>();
            flash.TimeRemaining = 0.15f;
            flash.FlashColor = { 1.0f, 0.35f, 0.35f, 1.0f };

            // Afegim (o reiniciem) el Knockback
            auto& knockback = target.AddOrReplaceComponent<KnockbackComponent>();
            knockback.TimeRemaining = 0.2f;

            // Apliquem la força física
            glm::vec2 targetPos = target.GetComponent<Runic2D::TransformComponent>().GetTranslation();
            glm::vec2 knockbackDir = glm::normalize(targetPos - sourcePos);

            if (target.HasComponent<Runic2D::Rigidbody2DComponent>()) {
                auto& rb = target.GetComponent<Runic2D::Rigidbody2DComponent>();
                if (B2_IS_NON_NULL(rb.RuntimeBody)) {
                    b2Body_ApplyLinearImpulse(rb.RuntimeBody, { knockbackDir.x * 1.5f, knockbackDir.y * 1.5f }, { 0,0 }, true);

                    // Treure el MaskBit d'enemics perquè no xoqui amb altres (el KnockbackSystem ja el tornarà a posar!)
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
            }

            // Crear el text de dany
            EntityFactory::CreateDamageText(targetPos, amount, false);
        }
    };
}