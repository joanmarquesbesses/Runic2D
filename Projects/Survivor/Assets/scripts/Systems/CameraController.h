#pragma once

#include "Runic2D.h"

using namespace Runic2D;

class CameraController : public Runic2D::ScriptableEntity
{
public:
    void OnUpdate(Timestep ts) override;

private:
    Entity m_Player;
    float m_SmoothSpeed = 5.0f;
    glm::vec3 m_Offset = { 0.0f, 0.0f, 0.0f };
};