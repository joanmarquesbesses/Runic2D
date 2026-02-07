#pragma once
#include "Runic2D.h"

#include "Enemy.h"

using namespace Runic2D;

class OrbitalProjectile : public ScriptableEntity {
public:
    float Damage = 10.0f;

    void OnCreate() override {
		player = GetScene()->GetEntityWithComponent<PlayerStatsComponent>();

        if (!HasComponent<Rigidbody2DComponent>())
        {
            auto& rb = GetEntity().AddComponent<Rigidbody2DComponent>();
            rb.Type = Rigidbody2DComponent::BodyType::Kinematic;
			rb.FixedRotation = true;
        }
        if (!HasComponent<CircleCollider2DComponent>()) {
            auto& cc = GetEntity().AddComponent<CircleCollider2DComponent>();
            cc.Radius = 0.5f;
            cc.IsSensor = true;
            cc.EnableSensorEvents = true;

            cc.CategoryBits = PhysicsLayers::Projectile;
            cc.MaskBits = PhysicsLayers::Enemy;

			GetScene()->InstantiatePhysics(GetEntity());
        }

        auto& anim = GetEntity().AddComponent<AnimationComponent>();

        Runic2D::AnimationProfile spin;
        spin.Name = "Spin";
        spin.AtlasTexture = GetEntity().GetComponent<SpriteRendererComponent>().Texture;
        spin.TileSize = { 100.0f, 100.0f };

        spin.StartFrame = 0;
        spin.FrameCount = 61;
        spin.FramesPerRow = 8;

        spin.FrameTime = 0.01f;
        spin.Loop = true;

        anim.Profiles.push_back(spin);

        anim.CurrentStateName = "Spin";
        anim.Playing = true;
        anim.Loop = true;

        anim.CurrentAnimation = Runic2D::Animation2D::CreateFromAtlas(
            spin.AtlasTexture,
            spin.TileSize,
            { 0.0f, 0.0f },
            spin.FrameCount,
            spin.FramesPerRow,
            spin.FrameTime
        );
        anim.Animations["Spin"] = anim.CurrentAnimation;
        anim.CurrentFrameIndex = (int)Runic2D::Random::Range(0.0f, (float)spin.FrameCount - 1);
    }

    void OnSensor(Entity other) {
        if (other.HasComponent<EnemyStatsComponent>())
        {
            if (other.HasComponent<NativeScriptComponent>())
            {
                auto& nsc = other.GetComponent<NativeScriptComponent>();
                Enemy* enemyScript = static_cast<Enemy*>(nsc.Instance);
                if (enemyScript)
                {
                    if (!player) {
                        player = GetScene()->GetEntityWithComponent<PlayerStatsComponent>();
                    }

                    glm::vec2 pushOrigin;

                    if (player) {
                        pushOrigin = player.GetComponent<TransformComponent>().Translation;
                    }
                    else {
                        pushOrigin = GetComponent<TransformComponent>().Translation;
                    }

                    enemyScript->TakeDamage(1, pushOrigin);

                    return;
                }
            }
        }
    }

private:
    Entity player;
};