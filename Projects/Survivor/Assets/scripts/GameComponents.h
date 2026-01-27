#pragma once

#include <glm/glm.hpp>

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

	// Opcional: Per si vols fer l'efecte imant més endavant
	bool Magnetized = false;
	float MoveSpeed = 5.0f;
};

struct PlayerStatsComponent {
	float Health = 100.0f;
	float MaxHealth = 100.0f;
	int Experience = 0;
	int Level = 1;
	void AddExperience(int amount) {
		Experience += amount;
		// Opcional: Comprovar si puja de nivell
	}
};