#include "GameplayLayer.h"

using namespace Runic2D;

GameplayLayer::GameplayLayer(Ref<Scene> scene)
    : Layer("Sandbox2D"), m_ActiveScene(scene) 
{
}

void GameplayLayer::OnAttach()
{
	R2D_PROFILE_FUNCTION();

    std::string projectPath = "Projects/Survivor/Survivor.r2dproj";

    if (Project::Load(projectPath))
    {
		R2D_INFO("Sandbox2D: Project carregat correctament des de {0}", projectPath);
        if (Project::LoadRuntimeLibrary())
        {
            R2D_INFO("GameplayLayer: DLL del joc carregada i inicialitzada.");
        }
        else
        {
            R2D_ERROR("GameplayLayer: Error carregant la DLL!");
            return;
        }
    }
    else
    {
        R2D_ERROR("Sandbox2D: No s'ha pogut carregar el projecte a {0}", projectPath);
    }

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
        m_ActiveScene->OnUpdateRunTime(ts); // Això mourà els scripts de la DLL!
        m_ActiveScene->OnRenderRuntime();

        if (m_ShowPhysicsColliders)
            ShowColliderOverlay();
    }
}

void GameplayLayer::OnImGuiRender()
{
	R2D_PROFILE_FUNCTION();
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
