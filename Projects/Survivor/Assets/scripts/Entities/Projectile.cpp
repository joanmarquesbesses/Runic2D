#include "Projectile.h"
#include "Enemy.h"

#include "Player.h"

void Projectile::OnCreate()
{
    if (HasComponent<Rigidbody2DComponent>())
        m_Rb = &GetComponent<Rigidbody2DComponent>();

    if (HasComponent<SpriteRendererComponent>())
    {
        m_SpriteRenderer = &GetComponent<SpriteRendererComponent>();
        if (m_SpriteRenderer->Texture)
            m_SpriteRenderer->SubTexture = nullptr;
            m_Texture = m_SpriteRenderer->Texture;
    }

    if (HasComponent<ProjectileComponent>())
    {
        auto& data = GetComponent<ProjectileComponent>();

        this->Speed = data.Speed;
        this->LifeTime = data.LifeTime;
    }
}

void Projectile::OnUpdate(Timestep ts)
{
    m_TimeAlive += ts;
    if (m_TimeAlive > LifeTime)
    {
        Destroy(); 
        return;
    }

    m_TimeSinceLastEmit += ts;
    if (m_TimeSinceLastEmit >= m_EmissionRate) {
        if (HasComponent<TransformComponent>())
        {
            auto& transform = GetComponent<TransformComponent>();

            ParticleProps props;
            props.Position = transform.Translation;
            //props.Position.z -= 0.1f;

            props.Velocity = { 0.0f, 0.0f };
            props.VelocityVariation = { 0.5f, 0.5f };
            props.LifeTime = 0.2f;
            props.ColorBegin = { 0.0f, 0.7f, 1.0f, 1.0f };
            props.ColorEnd = { 0.0f, 0.0f, 1.0f, 0.0f };
            props.SizeBegin = 0.2f;
            props.SizeEnd = 0.0f;
            props.SizeVariation = 0.05f;

            GetScene()->EmitParticles(props);
            m_TimeSinceLastEmit = 0.0f;
        }
    }

    m_Rb = &GetComponent<Rigidbody2DComponent>();
    b2BodyId bodyId = (b2BodyId)m_Rb->RuntimeBody;
    b2Rot rotation = b2Body_GetRotation(bodyId);
    glm::vec2 direction = { rotation.c, rotation.s };
    b2Vec2 vel = { direction.x * Speed, direction.y * Speed };
    b2Body_SetLinearVelocity(bodyId, vel);
}

void Projectile::OnSensor(Entity other)
{
    if (Owner == OwnerType::Player && other.HasComponent<Player>())
        return; 

    if (other.HasComponent<EnemyStatsComponent>()) {
        if (std::find(m_HitList.begin(), m_HitList.end(), other) != m_HitList.end()) {
            return;
        }

        if (other.HasComponent<NativeScriptComponent>()) {
            auto& nsc = other.GetComponent<NativeScriptComponent>();
            if (nsc.Instance) {
                Enemy* enemy = (Enemy*)nsc.Instance;

                glm::vec2 myPos = GetComponent<TransformComponent>().Translation;

                float damage = 10.0f;
                if (HasComponent<ProjectileComponent>())
                    damage = GetComponent<ProjectileComponent>().Damage;

                enemy->TakeDamage(damage, myPos);
            }
        }

        m_HitList.push_back(other);

        auto& pc = GetComponent<ProjectileComponent>();

        if (pc.Penetration > 0) {
            pc.Penetration--;

            // Opcional: Reduir una mica el mal o la velocitat després de travessar?
            // pc.Damage *= 0.8f; 
        }
        else {
            Destroy();
        }
    }

       /* if (Owner == OwnerType::Enemy && other.HasComponent<EnemyAI>())
            return; */
}