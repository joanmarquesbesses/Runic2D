#include "Projectile.h"

#include "Player.h"

void Projectile::OnCreate()
{
    if (HasComponent<Rigidbody2DComponent>())
        m_Rb = &GetComponent<Rigidbody2DComponent>();
}

void Projectile::OnUpdate(Timestep ts)
{
    m_TimeAlive += ts;
    if (m_TimeAlive > LifeTime)
    {
        Destroy(); // Auto-destrucció per temps
        return;
    }

    if (m_Rb)
    {
        /*b2BodyId bodyId = (b2BodyId)m_Rb->RuntimeBody;

        float angle = b2Body_GetAngle(bodyId);

        glm::vec2 velocity = { cos(angle), sin(angle) };

        b2Vec2 vel = { velocity.x * Speed, velocity.y * Speed };
        b2Body_SetLinearVelocity(bodyId, vel);*/
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