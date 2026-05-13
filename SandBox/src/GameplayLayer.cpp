#include "GameplayLayer.h"

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
	scene->OnRenderUI();

    if (m_ShowPhysicsColliders)
        ShowColliderOverlay();

    if(scene->IsDebugOverlayEnabled())
		scene->OnRenderDebugOverlay();
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
			if (scene) scene->SetDebugOverlayEnabled(!scene->IsDebugOverlayEnabled());
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
			m_ShowPhysicsColliders = !m_ShowPhysicsColliders;
            return true;
        }
	}

    return false;
}

void GameplayLayer::ShowColliderOverlay()
{
    auto scene = SceneManager::GetActiveScene();
    if (!scene) return;

    Entity cam = scene->GetPrimaryCameraEntity();
    if (!cam) return;

    auto& camera = cam.GetComponent<CameraComponent>().Camera;
    auto& tc = cam.GetComponent<TransformComponent>();
    glm::mat4 vp = camera.GetProjection() * glm::inverse(tc.GetTransform());
    scene->OnRenderOverlay(vp);
}
