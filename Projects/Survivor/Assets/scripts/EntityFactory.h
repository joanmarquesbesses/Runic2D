#pragma once
#include "Runic2D.h"

class EntityFactory
{
public:
    static void Init(Runic2D::Scene* scene);
    static void Shutdown();

    static Runic2D::Entity CreatePlayerProjectile(glm::vec2 position, glm::vec2 direction);
    static Runic2D::Entity CreateExperienceGem(glm::vec2 pos, int amount);

    static Runic2D::Entity CreateDamageText(glm::vec2 pos, float damageAmount, bool isCritical = false);
    
    static Runic2D::Entity CreateBat(glm::vec2 pos, float difficultyMult = 1.0f);

private:
    static Runic2D::Entity CreateBaseEnemy(glm::vec2 pos, std::string name);

private:
    static Runic2D::Scene* s_Scene;
    static Runic2D::Ref<Runic2D::Texture2D> s_ProjectileTexture;

	static Runic2D::Ref<Runic2D::Texture2D> s_BatTexture;
	static Runic2D::Ref<Runic2D::Texture2D> s_BatDeathTexture;

    static Runic2D::Ref<Runic2D::Texture2D> s_WhiteGemTexture;
    static Runic2D::Ref<Runic2D::Texture2D> s_GreenGemTexture;
    static Runic2D::Ref<Runic2D::Texture2D> s_RedGemTexture;
    static Runic2D::Ref<Runic2D::Texture2D> s_PurpleGemTexture;
};