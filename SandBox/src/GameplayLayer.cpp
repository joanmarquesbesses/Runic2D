#include "GameplayLayer.h"

#include "Runic2D/Systems/DebugSystem.h"

using namespace Runic2D;

GameplayLayer::GameplayLayer()
    : Layer("GameplayLayer") 
{
}

void GameplayLayer::OnAttach()
{
	R2D_PROFILE_FUNCTION();

    SceneManager::LoadStartScene();
}

void GameplayLayer::OnDetach()
{
	R2D_PROFILE_FUNCTION();

    auto scene = SceneManager::GetActiveScene();
    if (scene) scene->OnRuntimeStop();
}

void GameplayLayer::OnFixedUpdate(Runic2D::Timestep ts)
{
	R2D_PROFILE_FUNCTION();
	auto scene = SceneManager::GetActiveScene();
	if (scene) scene->OnFixedUpdateRunTime(ts);
}

void GameplayLayer::OnUpdate(Runic2D::Timestep ts)
{
    R2D_PROFILE_FUNCTION();

    Renderer2D::ResetStats();

    auto scene = SceneManager::GetActiveScene();
    if (!scene) return;

    RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.2f, 1 });
    RenderCommand::Clear();

    scene->OnUpdateRunTime(ts);
    scene->OnRenderRuntime();
}

void GameplayLayer::OnEvent(Runic2D::Event& e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowResizeEvent>(R2D_BIND_EVENT_FN(GameplayLayer::OnWindowResize));
    dispatcher.Dispatch<KeyPressedEvent>(R2D_BIND_EVENT_FN(GameplayLayer::OnKeyPressed));
}

bool GameplayLayer::OnWindowResize(WindowResizeEvent& e)
{
    auto scene = SceneManager::GetActiveScene();
    if (scene) scene->OnViewportResize(e.GetWidth(), e.GetHeight());
    return false;
}

bool GameplayLayer::OnKeyPressed(KeyPressedEvent& e)
{
    switch(e.GetKeyCode()){
        case KeyCode::F1:
        {
            auto scene = SceneManager::GetActiveScene();
            if (scene) {
                auto debugSystem = scene->GetSystem<DebugSystem>();
                if (debugSystem) {
                    debugSystem->SetShowStats(!debugSystem->GetShowStats());
                }
            }
            return true;
        }
        case KeyCode::F2:
        {
            static bool s_Profiling = false;
            if (s_Profiling)
            {
                R2D_PROFILE_END_SESSION();
                R2D_INFO("Profiling: Sessió finalitzada.");
            }
            else
            {
                R2D_PROFILE_BEGIN_SESSION("Gameplay_Session", "");
                R2D_INFO("Profiling: Sessió iniciada.");
            }
            s_Profiling = !s_Profiling;
            return true;
        }
        case KeyCode::F3:
        {
            auto scene = SceneManager::GetActiveScene();
            if (scene) {
                auto debugSystem = scene->GetSystem<DebugSystem>();
                if (debugSystem) {
                    debugSystem->SetShowColliders(!debugSystem->GetShowColliders());
                }
            }
            return true;
        }
	}

    return false;
}