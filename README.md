# Runic2D Engine

![Language](https://img.shields.io/badge/language-C%2B%2B17-blue.svg)
![Graphics](https://img.shields.io/badge/graphics-OpenGL-green.svg)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey.svg)
![License](https://img.shields.io/badge/license-MIT-orange.svg)

**Runic2D** is a high-performance custom 2D game engine written in C++ designed with **Data-Oriented Design** principles. It features a strictly separated architecture (Core/Client), a custom ECS implementation using EnTT, and an abstract rendering API.

Currently used to develop a *Survivor-like* technical demo (Dogfooding).

<p align="center">
  <img src="URL_DEL_GIF_DE_L_EDITOR_AQUI.gif" alt="Runic2D Editor Interface" width="100%">
</p>

## ⚙️ Key Features

### 🔧 Architecture & Memory
* **Entity Component System (ECS):** Integrated **EnTT** for cache-friendly memory layout and systems decoupling.
* **Modular Game Loop:** Strict separation between `Update` (Simulation) and `Render` logic.
* **Resource Management:** RAII-based resource ownership using custom smart pointer wrappers to prevent memory leaks.
* **Native Scripting:** C++ scripting bridge allowing gameplay logic to interact seamlessly with the engine core.

### 🎨 Rendering & Graphics
* **Abstract Rendering API:** Hardware Abstraction Layer (HAL) decoupling the engine from OpenGL (extensible to Vulkan/DirectX).
* **Batch Rendering:** Automatic geometry batching to minimize draw calls.
* **2D Physics:** Integrated Box2D with custom debug visualization (colliders, bounds).
* **Editor Tools:** Built with **ImGui**, featuring Scene Hierarchy, Inspector, and Content Browser.

---

## 💻 Code Highlights (Interesting Files)

Here are some direct links to core systems implementations:

* **Memory & Allocations:** [See `Core/Core.h`](https://github.com/joanmarquesbesses/Runic2D/blob/main/Runic2D/src/Runic2D/Core/Core.h) - *Smart pointer wrappers and memory definitions.*
* **Renderer Architecture:** [See `Renderer/Renderer2D.cpp`](https://github.com/joanmarquesbesses/Runic2D/blob/main/Runic2D/src/Runic2D/Renderer/Renderer2D.cpp) - *Batch rendering logic and flush implementation.*
* **ECS Integration:** [See `Scene/Scene.cpp`](https://github.com/joanmarquesbesses/Runic2D/blob/main/Runic2D/src/Runic2D/Scene/Scene.cpp) - *How EnTT registers components and handles entity destruction.*
* **Core Architecture:** [See `Core/Application.cpp`](https://github.com/joanmarquesbesses/Runic2D/blob/main/Runic2D/src/Runic2D/Core/Application.cpp) - *Main loop implementing a LayerStack architecture to manage event propagation, variable timestep updates, and ImGui integration.*

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
