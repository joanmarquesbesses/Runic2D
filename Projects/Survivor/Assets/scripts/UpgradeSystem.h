#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

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
};

class UpgradeDatabase {
public:
    static std::vector<UpgradeDef> GetAllUpgrades() {
        return {
            { UpgradeType::MultiShot,   "Multi-Shot",   "Fire +1 additional projectile", {0.2f, 0.8f, 1.0f, 1.0f} }, 
            { UpgradeType::AttackSpeed, "Haste",        "Attack Speed +20%",             {1.0f, 1.0f, 0.2f, 1.0f} }, 
            { UpgradeType::Damage,      "Might",        "Damage +20%",                   {1.0f, 0.2f, 0.2f, 1.0f} }, 
            { UpgradeType::Speed,       "Swiftness",    "Movement Speed +15%",           {0.2f, 1.0f, 0.5f, 1.0f} }  
        };
    }

    static UpgradeDef GetRandomUpgrade() {
        auto all = GetAllUpgrades();
        int index = rand() % all.size();
        return all[index];
    }
};