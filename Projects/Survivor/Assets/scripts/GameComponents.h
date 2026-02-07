#pragma once

#include <glm/glm.hpp>
#include "UpgradeSystem.h"
#include "GameContext.h"

namespace PhysicsLayers {
	enum Layer : uint32_t {
		None = 0,
		Default = 1 << 0, // 1  (Parets, coses normals)
		Player = 1 << 1, // 2
		Enemy = 1 << 2, // 4
		Projectile = 1 << 3, // 8
		Item = 1 << 4  // 16 (XP, Potions)
	};
}

struct ProjectileComponent
{
	float Speed = 10.0f;
	float LifeTime = 2.0f;
	float Damage = 10.0f;
	glm::vec2 Direction = { 1.0f, 0.0f };
	ProjectileComponent() = default;
	ProjectileComponent(const ProjectileComponent&) = default;
};

struct EnemyStatsComponent {
	float Health = 100.0f;
	float MaxHealth = 100.0f;
	float Speed = 2.0f;
	float Damage = 10.0f;
	int XPDrop = 1;
};

struct ExperienceComponent {
	int Amount = 1;
	bool Magnetized = false;
	float MoveSpeed = 5.0f;
};

struct PlayerStatsComponent {
	float Health = 100.0f;
	float MaxHealth = 100.0f;

	void TakeDamage(float amount) {
		Health -= amount;
		if (Health < 0) Health = 0;
		GameContext::Get().UpdateHealth(Health, MaxHealth);
	}

	void Heal(float amount) {
		Health += amount;
		if (Health > MaxHealth) Health = MaxHealth;
		GameContext::Get().UpdateHealth(Health, MaxHealth);
	}

	void SyncToContext() {
		GameContext::Get().UpdateHealth(Health, MaxHealth);
	}
};

struct UpgradeComponent {
	UpgradeDef Data; 
	UpgradeComponent() = default;
	UpgradeComponent(const UpgradeDef& data) : Data(data) {}
};

struct PlayerUpgradesComponent {
	std::map<UpgradeType, int> Levels;

	int GetLevel(UpgradeType type) {
		if (Levels.find(type) != Levels.end()) {
			return Levels[type];
		}
		return 0;
	}

	void AddLevel(UpgradeType type) {
		Levels[type]++;
	}
};