#pragma once
#include "Runic2D/Scene/ScriptableEntity.h"

class Runic2D::Texture2D;
class Runic2D::ParticleSystem;

using namespace Runic2D;

namespace Survivor {

    class Projectile : public ScriptableEntity
    {
    public:
        void OnCreate() override;
        void OnUpdate(Timestep ts) override;
        void OnSensor(Entity other) override;
		void OnFixedUpdate(Timestep ts) override;

        float Speed = 10.0f;
        float LifeTime = 2.0f;

        enum class OwnerType { Player, Enemy };

        OwnerType Owner = OwnerType::Player;

        float m_TimeSinceLastEmit = 0.0f;
        float m_EmissionRate = 0.01f;

    private:
        Ref<Texture2D> m_Texture = nullptr;
        float m_TimeAlive = 0.0f;
        float m_AnimTimer = 0.0f;

        std::vector<Entity> m_HitList;

        ParticleSystem* particleSystem = nullptr;
    };

}
