#pragma once
#include "Runic2D.h"

using namespace Runic2D;

class Projectile : public ScriptableEntity
{
public:
    void OnCreate() override;
    void OnUpdate(Timestep ts) override;
    void OnSensor(Entity other) override;

    float Speed = 10.0f;
    float LifeTime = 2.0f;

    int FramesPerRow = 1;
    int TotalFrames = 1;
    glm::vec2 TileSize = { 32.0f, 32.0f }; 
    float FrameSpeed = 0.1f;

    enum class OwnerType { Player, Enemy };

    OwnerType Owner = OwnerType::Player;

    float m_TimeSinceLastEmit = 0.0f;
    float m_EmissionRate = 0.02f;

private:
    Rigidbody2DComponent* m_Rb = nullptr;
    SpriteRendererComponent* m_SpriteRenderer = nullptr;
	Ref<Texture2D> m_Texture = nullptr;
    float m_TimeAlive = 0.0f;
    float m_AnimTimer = 0.0f;
};