#pragma once
#include "Runic2D.h"

class EntityFactory
{
public:
    static void Init(Runic2D::Scene* scene);
    static void Shutdown();

    static Runic2D::Entity CreatePlayerProjectile(glm::vec2 position, glm::vec2 direction);

private:
    static Runic2D::Scene* s_Scene;
    static Runic2D::Ref<Runic2D::Texture2D> s_ProjectileTexture;
};