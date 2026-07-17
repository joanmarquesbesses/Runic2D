#include "SurvivorPCH.h"
#include "ScriptRegistry.h"

#include "Runic2D/Scripting/ScriptEngine.h" 
#include "Runic2D/Core/Base/Core.h"
#include "Runic2D/Scene/Components/ComponentRegistry.h"
#include "Runic2D/Core/Input/InputManager.h"

#include "Core/GameComponents.h"

#include <yaml-cpp/yaml.h>

#ifndef R2D_DIST
#include <imgui/imgui.h>
#endif

namespace Survivor
{

    extern "C" {
        __declspec(dllexport) void InitRuntimeModule() {
            UpgradeDatabase::Init();

			// Runcic2D necessita que li diguem quins scripts tenim disponibles i com fer el bind d'aquests scripts a les entitats.
            ScriptEngine::SetScriptBinder(ScriptRegistry::BindScript);

            Runic2D::InputManager::BindAction("Shoot", Runic2D::MouseButton::Left, 0);
            Runic2D::InputManager::BindAction("MoveUp", Runic2D::KeyCode::W, 0);
            Runic2D::InputManager::BindAction("MoveDown", Runic2D::KeyCode::S, 0);
            Runic2D::InputManager::BindAction("MoveLeft", Runic2D::KeyCode::A, 0);
            Runic2D::InputManager::BindAction("MoveRight", Runic2D::KeyCode::D, 0);

            // Jugador 2 (Teclat secundari / Coop Local - Índex 1)
            Runic2D::InputManager::BindAction("Shoot", Runic2D::KeyCode::RightControl, 1);
            Runic2D::InputManager::BindAction("MoveUp", Runic2D::KeyCode::Up, 1);
            Runic2D::InputManager::BindAction("MoveDown", Runic2D::KeyCode::Down, 1);
            Runic2D::InputManager::BindAction("MoveLeft", Runic2D::KeyCode::Left, 1);
            Runic2D::InputManager::BindAction("MoveRight", Runic2D::KeyCode::Right, 1);

#ifndef R2D_DIST
			// El editor necessita saber quins scripts hi ha disponibles per mostrar-los al desplegable de l'inspector.
            ScriptEngine::SetScriptNamesGetter(ScriptRegistry::GetScriptNames);
#endif
			// Li pasem al editor els components personalitzats que volem que apareguin a l'inspector.
            // 1. PLAYER STATS (Gameplay) 
            Runic2D::ComponentRegistry::Register({
                "Player Stats", "Gameplay",
                [](Runic2D::Entity e) { if (!e.HasComponent<PlayerStatsComponent>()) e.AddComponent<PlayerStatsComponent>(); },
                [](Runic2D::Entity e) { return e.HasComponent<PlayerStatsComponent>(); },
#ifndef R2D_DIST
                [](Runic2D::Entity e) {
                    auto& c = e.GetComponent<PlayerStatsComponent>();
                    ImGui::DragFloat("Health", &c.Health, 0.1f, 0.0f, 50.0f);
                    ImGui::DragFloat("Speed", &c.Speed, 0.05f, 0.1f, 10.0f);
                    ImGui::DragFloat("Damage", &c.Damage, 0.1f, 0.0f, 20.0f);
                    ImGui::DragInt("ProjectileCount", &c.ProjectileCount, 1, 1, 100); },
#else
                    nullptr,
#endif
                [](Runic2D::Entity e) { e.RemoveComponent<PlayerStatsComponent>(); },
                [](Runic2D::Entity src, Runic2D::Entity dst) {
                    dst.AddOrReplaceComponent<PlayerStatsComponent>(src.GetComponent<PlayerStatsComponent>());
                },

                // SERIALITZA YAML
                [](YAML::Emitter& out, Runic2D::Entity e) {
                    auto& c = e.GetComponent<PlayerStatsComponent>();
                    out << YAML::Key << "Health" << YAML::Value << c.Health;
                    out << YAML::Key << "Speed" << YAML::Value << c.Speed;
                    out << YAML::Key << "Damage" << YAML::Value << c.Damage;
                    out << YAML::Key << "ProjectileCount" << YAML::Value << c.ProjectileCount;
                },
                // DESERIALITZA YAML
                [](YAML::Node& node, Runic2D::Entity e) {
                    auto& c = e.AddComponent<PlayerStatsComponent>();
                    if (node["Health"]) c.Health = node["Health"].as<float>();
                    if (node["Speed"]) c.Speed = node["Speed"].as<float>();
                    if (node["Damage"]) c.Damage = node["Damage"].as<float>();
                    if (node["ProjectileCount"]) c.ProjectileCount = node["ProjectileCount"].as<int>();
                },
                // SERIALITZA BINARI
                [](Runic2D::BufferStreamWriter& out, Runic2D::Entity e) {
                    out.WriteRaw(e.GetComponent<PlayerStatsComponent>());
                },
                // DESERIALITZA BINARI
                [](Runic2D::BufferStreamReader& in, Runic2D::Entity e) {
                    PlayerStatsComponent c;
                    in.ReadRaw(c);
                    e.AddOrReplaceComponent<PlayerStatsComponent>(c);
                },
                false // És component del joc, no del motor
                });

            // 2. ENEMY (AI) 
            Runic2D::ComponentRegistry::Register({
                "Enemy Component", "AI",
                [](Runic2D::Entity e) { if (!e.HasComponent<EnemyStatsComponent>()) e.AddComponent<EnemyStatsComponent>(); },
                [](Runic2D::Entity e) { return e.HasComponent<EnemyStatsComponent>(); },
#ifndef R2D_DIST
                [](Runic2D::Entity e) {
                    auto& c = e.GetComponent<EnemyStatsComponent>();
                    ImGui::DragFloat("Speed", &c.Speed, 0.1f, 0.0f, 20.0f);
                    ImGui::DragFloat("Damage", &c.Damage, 0.5f, 0.0f, 100.0f);
                    ImGui::DragInt("XPDrop", &c.XPDrop, 1, 0, 1000); },
#else
                    nullptr,
#endif
                [](Runic2D::Entity e) { e.RemoveComponent<EnemyStatsComponent>(); },
                [](Runic2D::Entity src, Runic2D::Entity dst) {
                    dst.AddOrReplaceComponent<EnemyStatsComponent>(src.GetComponent<EnemyStatsComponent>());
                },

                // SERIALITZA YAML
                [](YAML::Emitter& out, Runic2D::Entity e) {
                    auto& c = e.GetComponent<EnemyStatsComponent>();
                    out << YAML::Key << "Speed" << YAML::Value << c.Speed;
                    out << YAML::Key << "Damage" << YAML::Value << c.Damage;
                    out << YAML::Key << "XPDrop" << YAML::Value << c.XPDrop;
                },
                // DESERIALITZA YAML
                [](YAML::Node& node, Runic2D::Entity e) {
                    auto& c = e.AddComponent<EnemyStatsComponent>();
                    if (node["Speed"]) c.Speed = node["Speed"].as<float>();
                    if (node["Damage"]) c.Damage = node["Damage"].as<float>();
                    if (node["XPDrop"]) c.XPDrop = node["XPDrop"].as<int>();
                },
                // SERIALITZA BINARI
                [](Runic2D::BufferStreamWriter& out, Runic2D::Entity e) {
                    out.WriteRaw(e.GetComponent<EnemyStatsComponent>());
                },
                // DESERIALITZA BINARI
                [](Runic2D::BufferStreamReader& in, Runic2D::Entity e) {
                    EnemyStatsComponent c;
                    in.ReadRaw(c);
                    e.AddOrReplaceComponent<EnemyStatsComponent>(c);
                },
                false // És component del joc, no del motor
                });

            R2D_CORE_INFO("Survivor DLL: Inicialitzada i dades carregades.");
        }
    }

    extern "C" {
        __declspec(dllexport) void ShutdownRuntimeModule() {
            UpgradeDatabase::Shutdown();
#ifndef R2D_DIST
            Runic2D::ComponentRegistry::Clear();
#endif
            R2D_CORE_INFO("Survivor DLL: Shutdown correcte!");
        }
    }

} // namespace Survivor
