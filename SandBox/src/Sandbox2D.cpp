#include "Sandbox2D.h"

using namespace Runic2D;

Sandbox2D::Sandbox2D() : Layer("Sandbox2D") {

}

void Sandbox2D::OnAttach()
{
	R2D_PROFILE_FUNCTION();

    m_ActiveScene = CreateRef<Scene>();

    std::string projectPath = "Projects/Arkanoid/Arkanoid.r2dproj";

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
    std::string scenePath = "Projects/Arkanoid/Assets/scenes/Level1.r2dscene";

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

void Sandbox2D::OnDetach()
{
	R2D_PROFILE_FUNCTION();

    if (m_ActiveScene)
        m_ActiveScene->OnRuntimeStop();
}

void Sandbox2D::OnUpdate(Runic2D::Timestep ts)
{
	R2D_PROFILE_FUNCTION();

    RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
    RenderCommand::Clear();

    if (m_ActiveScene)
    {
        m_ActiveScene->OnUpdateRunTime(ts);
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
}

bool Sandbox2D::OnWindowResize(WindowResizeEvent& e)
{
    if (m_ActiveScene)
    {
        m_ActiveScene->OnViewportResize(e.GetWidth(), e.GetHeight());
    }
    return false;
}