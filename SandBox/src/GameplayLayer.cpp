#include "GameplayLayer.h"

using namespace Runic2D;

GameplayLayer::GameplayLayer(Ref<Scene> scene)
    : Layer("Sandbox2D"), m_ActiveScene(scene) 
{
}

void GameplayLayer::OnAttach()
{
	R2D_PROFILE_FUNCTION();

    SceneSerializer serializer(m_ActiveScene);
    std::string scenePath = "Projects/Survivor/Assets/scenes/MainScene.r2dscene";

    if (serializer.Deserialize(scenePath))
    {
        auto view = m_ActiveScene->GetAllEntitiesWith<NativeScriptComponent>();
        for (auto e : view)
        {
            Entity entity = { e, m_ActiveScene.get() };
            auto& nsc = entity.GetComponent<NativeScriptComponent>();

            if (!nsc.ClassName.empty())
            {
                ScriptEngine::BindScript(nsc.ClassName, entity);
            }
        }

        m_ActiveScene->OnRuntimeStart();

        auto& window = Application::Get().GetWindow();
        m_ActiveScene->OnViewportResize(window.GetWidth(), window.GetHeight());
    }
    else
    {
        R2D_ERROR("Sandbox2D: Could not load scene at {0}", scenePath);
    }
}

void GameplayLayer::OnDetach()
{
	R2D_PROFILE_FUNCTION();

    if (m_ActiveScene)
        m_ActiveScene->OnRuntimeStop();
}

void GameplayLayer::OnUpdate(Runic2D::Timestep ts)
{
    R2D_PROFILE_FUNCTION();

    RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.2f, 1 });
    RenderCommand::Clear();

    if (m_ActiveScene) {
        m_ActiveScene->OnUpdateRunTime(ts); 
        m_ActiveScene->OnRenderRuntime();
		m_ActiveScene->OnRenderUI();

        if (m_ShowPhysicsColliders)
            ShowColliderOverlay();
    }
}

void GameplayLayer::OnEvent(Runic2D::Event& e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowResizeEvent>(R2D_BIND_EVENT_FN(GameplayLayer::OnWindowResize));
    dispatcher.Dispatch<KeyPressedEvent>(R2D_BIND_EVENT_FN(GameplayLayer::OnKeyPressed));
}

bool GameplayLayer::OnWindowResize(WindowResizeEvent& e)
{
    if (m_ActiveScene)
    {
        m_ActiveScene->OnViewportResize(e.GetWidth(), e.GetHeight());
    }
    return false;
}

bool GameplayLayer::OnKeyPressed(KeyPressedEvent& e)
{
    switch(e.GetKeyCode()){
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
    glm::mat4 viewProj;

    Entity cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
    if (cameraEntity)
    {
        auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
        auto& tc = cameraEntity.GetComponent<TransformComponent>();
        viewProj = camera.GetProjection() * glm::inverse(tc.GetTransform());
    }

    m_ActiveScene->OnRenderOverlay(viewProj);
}
