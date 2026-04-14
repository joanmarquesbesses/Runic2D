#pragma once

#include <glm/glm.hpp>
#include "UpgradeSystem.h"
#include "GameContext.h"

namespace PhysicsLayers {
	enum Layer : uint16_t { 
		None = 0,
		Default = 1 << 0,
		Player = 1 << 1,
		Projectile = 1 << 2,
		Item = 1 << 3,
		Enemy = 1 << 4,
		EnemyBat = 1 << 5, 
		EnemySlime = 1 << 6  
	};
}

struct ProjectileComponent
{
	float Speed = 10.0f;
	float LifeTime = 2.0f;
	float Damage = 10.0f;
	glm::vec2 Direction = { 1.0f, 0.0f };
	int Penetration = 0;
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

struct GameStatsComponent {
	float TimeAlive = 0.0f;
	GameState State = GameState::Running;

	// Progression data
	int CurrentLevel = 1;
	float CurrentXP = 0.0f;
	float MaxXP = 100.0f;

	// Stats que la UI necessita consultar sovint
	float PlayerHealth = 100.0f;
	float PlayerMaxHealth = 100.0f;

	// Callbacks (ara dins del component)
	std::function<void(int)> OnLevelUp;
	std::function<void(float, float)> OnHealthChanged;
	std::function<void(float, float)> OnXPChanged;
	std::function<void(UpgradeType)> OnUpgradeApplied;

	// Mètodes d'ajuda (helper functions)
	void AddXP(float amount) {
		CurrentXP += amount;
		if (CurrentXP >= MaxXP) {
			CurrentXP -= MaxXP;
			CurrentLevel++;
			MaxXP *= 1.2f;
			State = GameState::LevelUp;
			if (OnLevelUp) OnLevelUp(CurrentLevel);
		}
		if (OnXPChanged) OnXPChanged(CurrentXP, MaxXP);
	}
};