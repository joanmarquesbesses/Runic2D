#include "Sandbox2D.h"

#include "../../Projects/Survivor/Assets/scripts/EntityFactory.h"

using namespace Runic2D;

Sandbox2D::Sandbox2D(Runic2D::Ref<GameContext> context) : Layer("Sandbox2D"), m_Context(context) {

}

void Sandbox2D::OnAttach()
{
	R2D_PROFILE_FUNCTION();

    m_ActiveScene = CreateRef<Scene>();

    std::string projectPath = "Projects/Survivor/Survivor.r2dproj";

    if (Project::Load(projectPath))
    {
		R2D_INFO("Sandbox2D: Project carregat correctament des de {0}", projectPath);
    }
    else
    {
        // Si falles aquí, tot petarà després, millor avisar
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
        EntityFactory::Init(m_ActiveScene.get());

        auto& window = Application::Get().GetWindow();
        m_ActiveScene->OnViewportResize(window.GetWidth(), window.GetHeight());
      
        m_TextFPS = m_ActiveScene->CreateEntity("Text FPS");
        auto& tc = m_TextFPS.GetComponent<TransformComponent>();
        tc.Translation = { -8.0f, 4.0f, 10.0f };
        tc.Scale = { 1.0f, 1.0f, 1.0f };

        auto& textComp = m_TextFPS.AddComponent<TextComponent>();
        textComp.TextString = "FPS: 0";
        textComp.Color = { 0.0f, 1.0f, 0.0f, 1.0f }; 
        textComp.Kerning = 0.0f;
        textComp.LineSpacing = 0.0f;
		textComp.Visible = showFPS;
    }
    else
    {
        R2D_ERROR("Sandbox2D: Could not load scene at {0}", scenePath);
    }
}

void Sandbox2D::OnDetach()
{
	R2D_PROFILE_FUNCTION();

    if (m_ActiveScene)
        m_ActiveScene->OnRuntimeStop();
}

void Sandbox2D::OnUpdate(Runic2D::Timestep ts)
{
	R2D_PROFILE_FUNCTION();

    RenderCommand::SetClearColor({ 0.2f, 0.2f, 0.2f, 1 });
    RenderCommand::Clear();

    if (m_Context && m_ActiveScene) {
        if (m_Context->State == GameState::Running) {
            m_ActiveScene->OnUpdateRunTime(ts); 
            m_Context->TimeAlive += ts;         
        }
        else {
            m_ActiveScene->OnRenderRuntime();
        }

        if (m_ShowPhysicsColliders)
        {
            ShowColliderOverlay();
        }
    }   
}

void Sandbox2D::OnImGuiRender()
{
	R2D_PROFILE_FUNCTION();
}

void Sandbox2D::OnEvent(Runic2D::Event& e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowResizeEvent>(R2D_BIND_EVENT_FN(Sandbox2D::OnWindowResize));
    dispatcher.Dispatch<KeyPressedEvent>(R2D_BIND_EVENT_FN(Sandbox2D::OnKeyPressed));
}

bool Sandbox2D::OnWindowResize(WindowResizeEvent& e)
{
    if (m_ActiveScene)
    {
        m_ActiveScene->OnViewportResize(e.GetWidth(), e.GetHeight());
    }
    return false;
}

bool Sandbox2D::OnKeyPressed(KeyPressedEvent& e)
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

void Sandbox2D::ShowColliderOverlay()
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
