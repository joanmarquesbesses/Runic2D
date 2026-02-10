#include "UpgradeSystem.h"

#include "Runic2D/Assets/ResourceManager.h"
#include "Runic2D/Project/Project.h"

// card base textures
Ref<Texture2D> UpgradeDatabase::s_CardBaseTexture_Multishot = nullptr;
Ref<Texture2D> UpgradeDatabase::s_CardBaseTexture_AttackSpeed = nullptr;
Ref<Texture2D> UpgradeDatabase::s_CardBaseTexture_Damage = nullptr;
Ref<Texture2D> UpgradeDatabase::s_CardBaseTexture_Speed = nullptr;
Ref<Texture2D> UpgradeDatabase::s_CardBaseTexture_HealthRegen = nullptr;

// gameplay textures
Ref<Texture2D> UpgradeDatabase::s_Orbital_Texture = nullptr;

void UpgradeDatabase::Init() {
	// Load card base textures
    s_CardBaseTexture_Multishot = ResourceManager::Get<Texture2D>(Project::GetAssetFileSystemPath("textures/upgrades/upgrade1.png"));
    s_CardBaseTexture_AttackSpeed = ResourceManager::Get<Texture2D>(Project::GetAssetFileSystemPath("textures/upgrades/upgrade2.png"));
    s_CardBaseTexture_Damage = ResourceManager::Get<Texture2D>(Project::GetAssetFileSystemPath("textures/upgrades/upgrade3.png"));
    s_CardBaseTexture_Speed = ResourceManager::Get<Texture2D>(Project::GetAssetFileSystemPath("textures/upgrades/upgrade4.png"));
    s_CardBaseTexture_HealthRegen = ResourceManager::Get<Texture2D>(Project::GetAssetFileSystemPath("textures/upgrades/upgrade5.png"));

	// Load gameplay textures
	s_Orbital_Texture = ResourceManager::Get<Texture2D>(Project::GetAssetFileSystemPath("textures/upgrades/orbital/orbital.png"));
}

void UpgradeDatabase::Shutdown() {
    s_CardBaseTexture_Multishot = nullptr;
    s_CardBaseTexture_AttackSpeed = nullptr;
    s_CardBaseTexture_Damage = nullptr;
    s_CardBaseTexture_Speed = nullptr;
    s_CardBaseTexture_HealthRegen = nullptr;
}

std::vector<UpgradeDef> UpgradeDatabase::GetAllUpgrades() {
    return {
        {
            UpgradeType::MultiShot,
            "Multi-Shot",
            "Fire +1 additional\nprojectile",
            {0.2f, 0.8f, 1.0f, 1.0f}, 
            s_CardBaseTexture_Multishot,
            nullptr           
        },
        {
            UpgradeType::AttackSpeed,
            "Haste",
            "Attack Speed +20%",
            {1.0f, 1.0f, 0.2f, 1.0f}, 
            s_CardBaseTexture_AttackSpeed,
            nullptr
        },
        {
            UpgradeType::Damage,
            "Might",
            "Damage +20%",
            {1.0f, 0.2f, 0.2f, 1.0f}, 
            s_CardBaseTexture_Damage,
            nullptr
        },
        {
            UpgradeType::Speed,
            "Swiftness",
            "Movement Speed\n+15%",
            {0.2f, 1.0f, 0.5f, 1.0f},
            s_CardBaseTexture_Speed,
            nullptr
        },
        {
            UpgradeType::Piercing,
            "Piercing",
            "Projectile penetration",
            {0.2f, 1.0f, 0.5f, 1.0f},
            s_CardBaseTexture_HealthRegen,
            nullptr
        },
        {
            UpgradeType::Orbitals,
            "Orbitals",
            "Orbitals",
            {0.2f, 1.0f, 0.5f, 1.0f},
            s_CardBaseTexture_HealthRegen,
            nullptr
        }
    };
}

std::vector<UpgradeDef> UpgradeDatabase::GetRandomUniqueUpgrades(int count) {
    std::vector<UpgradeDef> pool = GetAllUpgrades();
    if (count >= pool.size()) return pool;
    std::shuffle(pool.begin(), pool.end(), Runic2D::Random::GetEngine());
    pool.resize(count);
    return pool;
}