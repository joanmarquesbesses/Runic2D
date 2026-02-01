#include "UILayer.h"

#include "../../Projects/Survivor/Assets/scripts/EntityFactory.h"
#include "../../Projects/Survivor/Assets/scripts/HUDManager.h"

using namespace Runic2D;

UILayer::UILayer(Ref<GameContext> context) : Layer("UILayer"), m_Context(context) {

}

void UILayer::OnAttach()
{
	R2D_PROFILE_FUNCTION();

    m_ActiveScene = CreateRef<Scene>();
    m_ActiveScene->OnRuntimeStart();

    Entity cam = m_ActiveScene->CreateEntity("UICamera");
    auto& cc = cam.AddComponent<CameraComponent>();
    cc.Camera.SetOrthographic(10.0f, -1.0f, 1.0f);
    
    Entity hudManager = m_ActiveScene->CreateEntity("HUD_Manager");
    hudManager.AddComponent<NativeScriptComponent>().Bind<HUDManager>();

    // Debug
    m_TextFPS = m_ActiveScene->CreateEntity("FPSCounter");
    auto& tc = m_TextFPS.GetComponent<TransformComponent>();
    tc.Translation = { -8.5f, 4.5f, 0.1f };
	tc.Scale = { 0.50f, 0.50f, 1.0f };

    auto& txtFPS = m_TextFPS.AddComponent<TextComponent>();
    txtFPS.TextString = "FPS: 0";
    txtFPS.Color = { 0.0f, 1.0f, 0.0f, 1.0f };
	txtFPS.Visible = showFPS;

    auto& window = Application::Get().GetWindow();
    m_ActiveScene->OnViewportResize(window.GetWidth(), window.GetHeight());
}

void UILayer::OnDetach()
{
	R2D_PROFILE_FUNCTION();

    if (m_ActiveScene)
        m_ActiveScene->OnRuntimeStop();
}

void UILayer::OnUpdate(Runic2D::Timestep ts)
{
	R2D_PROFILE_FUNCTION();

    RenderCommand::ClearDepth();

    if (m_ActiveScene)
    {
        if (showFPS)
        {
            auto& txt = m_TextFPS.GetComponent<TextComponent>();
            txt.TextString = "FPS: " + std::to_string((int)Application::Get().GetAverageFPS());
        }

        m_ActiveScene->OnUpdateRunTime(ts);
    }

}

void UILayer::OnImGuiRender()
{
	R2D_PROFILE_FUNCTION();
}

void UILayer::OnEvent(Runic2D::Event& e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowResizeEvent>(R2D_BIND_EVENT_FN(UILayer::OnWindowResize));
    dispatcher.Dispatch<KeyPressedEvent>(R2D_BIND_EVENT_FN(UILayer::OnKeyPressed));
    dispatcher.Dispatch<MouseButtonPressedEvent>(R2D_BIND_EVENT_FN(UILayer::OnMouseButtonPressed));
}

bool UILayer::OnWindowResize(WindowResizeEvent& e)
{
    if (m_ActiveScene)
    {
        m_ActiveScene->OnViewportResize(e.GetWidth(), e.GetHeight());
    }
    return false;
}

bool UILayer::OnKeyPressed(KeyPressedEvent& e)
{
    switch(e.GetKeyCode()){
        case KeyCode::F1:
        {
            showFPS = !showFPS;
            auto& txt = m_TextFPS.GetComponent<TextComponent>();
            txt.Visible = showFPS;
            return true;
        }
        case KeyCode::F2: 
        {
            auto& window = Application::Get().GetWindow();
            bool vsyncState = window.IsVSync();
            window.SetVSync(!vsyncState);
            return true;
        }
	}

    return false;
}

bool UILayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
{
    if (m_Context && m_Context->State == GameState::LevelUp)
    {
        return true;
    }

    return false; 
}
