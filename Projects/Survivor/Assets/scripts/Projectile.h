#pragma once
#include "Runic2D.h"

using namespace Runic2D;

class Projectile : public ScriptableEntity
{
public:
    void OnCreate() override;
    void OnUpdate(Timestep ts) override;
    void OnCollision(Entity other) override;

    float Speed = 10.0f;
    float LifeTime = 2.0f;

    enum class OwnerType { Player, Enemy };

    OwnerType Owner = OwnerType::Player;

private:
    Rigidbody2DComponent* m_Rb = nullptr;
    AnimationComponent* m_Anim = nullptr;
    TransformComponent* m_Transform = nullptr;
    float m_TimeAlive = 0.0f;
};