#pragma once
#include "Runic2D.h"
#include "OrbitalProjectile.h"
#include <vector>
#include <Box2D/box2d.h>

using namespace Runic2D;

class OrbitalManager : public ScriptableEntity {
public:
    float RotationSpeed = 60.0f; 
    float Radius = 2.5f;    
    float m_CurrentAngle = 0.0f;

    std::vector<Entity> m_Orbitals; 

    void OnCreate() override {
		PlayerEntity = GetScene()->GetEntityWithComponent<PlayerStatsComponent>();
	}

    void OnUpdate(Timestep ts) override {

        int count = (int)m_Orbitals.size();
        if (count == 0) return;

        if (!PlayerEntity)
        {
            PlayerEntity = GetScene()->FindEntityByName("Player");
            if (!PlayerEntity) return;
		}

        glm::vec2 centerPos = { PlayerEntity.GetComponent<TransformComponent>().Translation.x,
                                PlayerEntity.GetComponent<TransformComponent>().Translation.y };

        m_CurrentAngle += RotationSpeed * ts;
        if (m_CurrentAngle > 360.0f) m_CurrentAngle -= 360.0f;

        float angleStep = (2.0f * 3.14159f) / (float)count; 

        for (int i = 0; i < count; i++) {
            Entity orbital = m_Orbitals[i];
            if (!orbital) continue;

            float ballAngle = glm::radians(m_CurrentAngle) + (i * angleStep);

            float x = centerPos.x + cos(ballAngle) * Radius;
            float y = centerPos.y + sin(ballAngle) * Radius;

            auto& tc = orbital.GetComponent<TransformComponent>();
            bool bodyMoved = false;

            if (orbital.HasComponent<Rigidbody2DComponent>()) {
                auto& rb = orbital.GetComponent<Rigidbody2DComponent>();
                b2BodyId bodyId = (b2BodyId)rb.RuntimeBody;
                if (B2_IS_NON_NULL(bodyId)) {
                    b2Vec2 newPos = { x, y };
                    b2Body_SetTransform(bodyId, newPos, b2MakeRot(0.0f));
                    bodyMoved = true;
                }
                if (!bodyMoved) {
                    tc.SetTranslation({ x, y, 0.1f });
                }
            }
        }
    }

    void OnDestroy() override {
        for (auto orbital : m_Orbitals) {
            if (orbital) GetScene()->SubmitForDestruction(orbital);
        }
        m_Orbitals.clear();
    }

    void SetLevel(int level) {
        int count = level + 1;
        RebuildOrbitals(count);
    }

private:

	Entity PlayerEntity;

    void RebuildOrbitals(int count) {
        for (auto orbital : m_Orbitals) {
            if (orbital) GetScene()->DestroyEntity(orbital);
        }
        m_Orbitals.clear();

        for (int i = 0; i < count; i++) {
            Entity orbital = GetScene()->CreateEntity("OrbitalBall");

            auto& tc = orbital.GetComponent<TransformComponent>();
            tc.SetScale({ 1.0f, 1.0f, 1.0f });

            auto& sprite = orbital.AddComponent<SpriteRendererComponent>();
			sprite.Texture = UpgradeDatabase::GetOrbitalTexture();

            orbital.AddComponent<NativeScriptComponent>().Bind<OrbitalProjectile>();

            m_Orbitals.push_back(orbital);
        }
    }
};