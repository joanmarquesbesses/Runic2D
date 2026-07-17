#pragma once

#include <glm/glm.hpp>
#include "Systems/UpgradeDatabase.h"

#include "Runic2D/Scene/Entity.h"

namespace Survivor {

	enum class GameState {
		Running = 0,
		Paused,
		LevelUp,
		GameOver
	};

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
		float Health		= 100.0f;
		float MaxHealth		= 100.0f;
		float HealthRegen	= 0.0f;
		float Speed			= 5.0f;
		float Damage		= 10.0f;
		float AttackSpeed	= 1.0f;
		int   ProjectileCount = 1;
		bool  Piercing		= false;
	};

	struct UpgradeComponent {
		UpgradeDef Data;
		UpgradeComponent() = default;
		UpgradeComponent(const UpgradeDef& data) : Data(data) {}
		Entity TitleEntity = {};
		Entity DescEntity = {};
	};

	struct PlayerUpgradesComponent {
		std::map<UpgradeType, int> Levels;

		void AddLevel(UpgradeType type) { Levels[type]++; }

		int GetLevel(UpgradeType type) const {
			auto it = Levels.find(type);
			return it != Levels.end() ? it->second : 0;
		}
	};

	struct GameStatsComponent {
		GameState State = GameState::Running;

		float TimeAlive  = 0.0f;
		float CurrentXP  = 0.0f;
		float MaxXP		 = 100.0f;
		int CurrentLevel = 1;

		std::function<void(UpgradeType)> OnUpgradeApplied;

		// Mètodes d'ajuda (helper functions)
		void AddXP(float amount) {
			CurrentXP += amount;
			if (CurrentXP >= MaxXP) {
				CurrentXP -= MaxXP;
				CurrentLevel++;
				MaxXP *= 1.2f;
				State = GameState::LevelUp;
			}
		}
	};

	struct MovementComponent {
		float speed = 0.0f;
		glm::vec2 direction{ 0.0f };
	};

	struct DamageFlashComponent {
		float TimeRemaining = 0.0f;
		glm::vec4 FlashColor = { 1.0f, 0.35f, 0.35f, 1.0f };
	};

	struct KnockbackComponent {
		float TimeRemaining = 0.0f;
	};

	struct DeadTag {
		bool dummy = true;
	};

}
