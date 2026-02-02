#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "Runic2D/Utils/Random.h" 
#include <algorithm> 
#include "Runic2D/Renderer/Texture.h"

using namespace Runic2D;

enum class UpgradeType {
    None = 0,
    MultiShot,    
    AttackSpeed,  
    Damage,       
    Speed,        
    HealthRegen   
};

struct UpgradeDef {
    UpgradeType Type;
    std::string Title;
    std::string Description;
    glm::vec4 Color;
    Ref<Texture2D> CardTexture;
	Ref<Texture2D> Icon;
};

class UpgradeDatabase {
public:
    static void Init();
    static void Shutdown();

    static std::vector<UpgradeDef> GetAllUpgrades();
    static std::vector<UpgradeDef> GetRandomUniqueUpgrades(int count);

private:
    static Ref<Texture2D> s_CardBaseTexture_Multishot;
    static Ref<Texture2D> s_CardBaseTexture_AttackSpeed;
    static Ref<Texture2D> s_CardBaseTexture_Damage;
    static Ref<Texture2D> s_CardBaseTexture_Speed;
    static Ref<Texture2D> s_CardBaseTexture_HealthRegen;
};