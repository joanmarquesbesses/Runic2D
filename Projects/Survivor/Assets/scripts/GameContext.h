#pragma once
#include <memory>

struct GameContext {
    static GameContext* s_Instance;

    GameContext() { s_Instance = this; }
    ~GameContext() { s_Instance = nullptr; }

    static GameContext& Get() { return *s_Instance; }

    float TimeAlive = 0.0f; 
};