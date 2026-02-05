# Runic2D Engine

![Language](https://img.shields.io/badge/language-C%2B%2B17-blue.svg)
![Graphics](https://img.shields.io/badge/graphics-OpenGL-green.svg)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)
![License](https://img.shields.io/badge/license-MIT-orange.svg)

**Runic2D** is a high-performance custom 2D game engine written in C++ designed with **Data-Oriented Design** principles. It features a strictly separated architecture (Core/Client), a custom ECS implementation using EnTT, and an abstract rendering API.
> 🚧 **Note:** This project is currently **in active development**. Features and API may change as optimization continues.

<p align="center">
  <img src="https://github.com/joanmarquesbesses/Runic2D/blob/main/docs/images/sceneeditor.gif" alt="Runic2D Editor Interface" width="100%">
</p>

## ⚙️ Key Features

### 🔧 Architecture & Memory
* **Entity Component System (ECS):** Integrated **EnTT** for cache-friendly memory layout and systems decoupling.
* **Layered Architecture:** Application logic organized in a stack of layers, allowing precise control over update order, event propagation, and overlay rendering (ImGui).
* **Resource Management:** RAII-based resource ownership using custom smart pointer aliases (`Ref`/`Scope`) to prevent memory leaks.
* **Native Scripting:** C++ scripting bridge allowing gameplay logic to interact seamlessly with the engine core.

### 🎨 Rendering & Graphics
* **Abstract Rendering API:** Hardware Abstraction Layer (HAL) decoupling the engine from OpenGL (extensible to Vulkan/DirectX).
* **Batch Rendering:** Automatic dynamic geometry batching to minimize draw calls and maximize throughput.
* **Advanced Text Rendering:** High-quality text rendering using MSDF (Multi-channel Signed Distance Field) font atlases with support for kerning and dynamic scaling without pixelation.
* **2D Physics:** Integrated Box2D with custom debug visualization (colliders, bounds) and event callbacks.
* **Editor Tools:** Built with **ImGui**, featuring Scene Hierarchy, Inspector, Content Browser, and Profiling tools.

---

## 🎮 Gameplay Integration (Survivor Demo)

To validate the engine's architecture, I am developing a **Survivor-like game** alongside the engine (*Dogfooding*). The gameplay logic is decoupled from the engine core using an event-driven approach.

The **Game Context** manages the global state and uses `std::function` callbacks to trigger UI events (like Level Up screens) or Upgrade applications without hardcoding dependencies.

```cpp
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

    // Game State Data
    float TimeAlive = 0.0f; 
    GameState State = GameState::Running;
    int CurrentLevel = 1;
    float CurrentXP = 0.0f;
    
    // Event Callbacks (Decoupled Logic)
    std::function<void(int)> OnLevelUp;
    std::function<void(UpgradeType)> OnUpgradeApplied;

    void TriggerLevelUp(int level) {
        // Pauses the game loop and triggers the UI Layer via callback
        State = GameState::LevelUp;
        if (OnLevelUp) OnLevelUp(level);
    }

    void TriggerUpgradeApplied(UpgradeType type) {
        if (OnUpgradeApplied) OnUpgradeApplied(type);
        State = GameState::Running; // Resume game
    }
};
```

## 💻 Code Highlights (Interesting Files)

Here are some direct links to core systems implementations:

* **Memory & Macros:** [See `Core/Core.h`](https://github.com/joanmarquesbesses/Runic2D/blob/main/Runic2D/src/Runic2D/Core/Core.h) - *Defines smart pointer aliases (`Ref`/`Scope`) for RAII compliance and platform-specific assertions.*
* **Renderer Architecture:** [See `Renderer/Renderer2D.cpp`](https://github.com/joanmarquesbesses/Runic2D/blob/main/Runic2D/src/Runic2D/Renderer/Renderer2D.cpp) - *Implements automatic **Batch Rendering**, texture slot management, *MSDF Text Rendering* and dynamic vertex buffer flushing.*
* **ECS & Systems:** [See `Scene/Scene.cpp`](https://github.com/joanmarquesbesses/Runic2D/blob/main/Runic2D/src/Runic2D/Scene/Scene.cpp) - *Handles entity lifecycle via EnTT, Box2D physics world integration, and runtime/editor update splits.*
* **Core Application:** [See `Core/Application.cpp`](https://github.com/joanmarquesbesses/Runic2D/blob/main/Runic2D/src/Runic2D/Core/Application.cpp) - *Main loop implementing a **LayerStack architecture** to manage event propagation, variable timestep updates, and ImGui integration.*

---

## 🛠️ Getting Started

### Prerequisites
* Visual Studio 2022
* Git

### Build Instructions
To build Runic2D from source, follow these steps:

1.  **Clone the repository:**
    Make sure to clone recursively to include submodules.
    ```bash
    git clone --recursive [https://github.com/joanmarquesbesses/Runic2D.git](https://github.com/joanmarquesbesses/Runic2D.git)
    ```

2.  **Setup Premake:**
    * Download the **Premake 5.0** binary (zip) from the [official website](https://premake.github.io/).
    * Extract and place the `premake5.exe` file inside the `vendor/bin/premake` directory (create the folder structure if it doesn't exist).

3.  **Generate Project Files:**
    Run the generation script located in the scripts folder:
    ```bash
    scripts/GenerateProjects.bat
    ```

4.  **Build & Run:**
    Open `Runic2D.sln` in Visual Studio. The build configuration depends on the target:
    
    * **To run the Editor:**
        * Set **`Runic2D-Editor`** as the *Startup Project* (Right-click -> Set as Startup Project).
        * Select **`Release`** configuration.
        
    * **To run the Game (Survivor Demo):**
        * Set **`Sandbox`** as the *Startup Project*.
        * Select **`Dist`** (Distribution) configuration for maximum performance.

---

## 📸 Gallery

| Physics Debug | Demo Gameplay |
| :---: | :---: |
| <img src="https://github.com/joanmarquesbesses/Runic2D/blob/main/docs/images/physics.gif" width="100%"> | <img src="https://github.com/joanmarquesbesses/Runic2D/blob/main/docs/images/gameplay.gif" width="100%"> |
