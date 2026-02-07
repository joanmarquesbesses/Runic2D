#pragma once
#include <functional>

#include "UpgradeSystem.h"

enum class GameState {
    Running,
    LevelUp,
    GameOver
};

struct GameContext {
    static GameContext* s_Instance;  

    GameContext() { s_Instance = this; }
    ~GameContext() { s_Instance = nullptr; }

    static GameContext& Get() { return *s_Instance; }

    // Global game state
    float TimeAlive = 0.0f; 
    GameState State = GameState::Running;

    // Player datafor UI
	float PlayerHealth = 100.0f;
	float PlayerMaxHealth = 100.0f;

	// Progression data
    int CurrentLevel = 1;
    float CurrentXP = 0.0f;
    float MaxXP = 100.0f;
    
	// Event callbacks
    std::function<void(int)> OnLevelUp;
    std::function<void(UpgradeType)> OnUpgradeApplied;
	std::function<void(float, float)> OnHealthChanged; // (currentHealth, maxHealth)
	std::function<void(float, float)> OnXPChanged; // (currentXP, maxXP)

    void TriggerLevelUp(int level) {
        State = GameState::LevelUp;
        if (OnLevelUp) OnLevelUp(level);
    }

    void TriggerUpgradeApplied(UpgradeType type) {
		State = GameState::Running;
        if (OnUpgradeApplied) OnUpgradeApplied(type);
    }

    void UpdateHealth(float currentHealth, float maxHealth) {
        PlayerHealth = currentHealth;
        PlayerMaxHealth = maxHealth;
        if (OnHealthChanged) OnHealthChanged(currentHealth, maxHealth);
	}

    void AddXP(float amount) {
        CurrentXP += amount;
        if (CurrentXP >= MaxXP) {
            CurrentXP -= MaxXP; 
            CurrentLevel++;
            MaxXP *= 1.2f;

            TriggerLevelUp(CurrentLevel);
        }

        if (OnXPChanged) OnXPChanged(CurrentXP, MaxXP);
	}
};