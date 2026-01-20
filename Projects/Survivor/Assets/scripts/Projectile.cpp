#include "Projectile.h"

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

    if(HasComponent<TransformComponent>())
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
    }

    if (m_Rb)
    {
        b2BodyId bodyId = (b2BodyId)m_Rb->RuntimeBody;
        b2Rot rotation = b2Body_GetRotation(bodyId);
        glm::vec2 direction = { rotation.c, rotation.s };
        b2Vec2 vel = { direction.x * Speed, direction.y * Speed };
        b2Body_SetLinearVelocity(bodyId, vel);
    }
}

void Projectile::OnCollision(Entity other)
{
    if (other.HasComponent<NativeScriptComponent>())
    {
        if (Owner == OwnerType::Player && other.HasComponent<Player>())
            return; 

       /* if (Owner == OwnerType::Enemy && other.HasComponent<EnemyAI>())
            return; */
    }
	Destroy(); 
}