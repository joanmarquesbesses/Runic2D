#pragma once
#include <functional>

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

    float TimeAlive = 0.0f; 
    GameState State = GameState::Running;

    int CurrentLevel = 1;
    float CurrentXP = 0.0f;
    float MaxXP = 100.0f;
        
    std::function<void(int)> OnLevelUp;

    void TriggerLevelUp(int level) {
        if (OnLevelUp) OnLevelUp(level);
    }
};