#include "UILayer.h"

#include "../../Projects/Survivor/Assets/scripts/EntityFactory.h"
#include "../../Projects/Survivor/Assets/scripts/HUDManager.h"

using namespace Runic2D;

UILayer::UILayer(Ref<GameContext> context) : Layer("UILayer"), m_Context(context) {

}

void UILayer::OnAttach()
{
	R2D_PROFILE_FUNCTION();

    UpgradeDatabase::Init();

    m_ActiveScene = CreateRef<Scene>();
    m_ActiveScene->OnRuntimeStart();

    Entity cam = m_ActiveScene->CreateEntity("UICamera");
    auto& cc = cam.AddComponent<CameraComponent>();
    cc.Camera.SetOrthographic(10.0f, -1.0f, 1.0f);
    
    Entity hudManager = m_ActiveScene->CreateEntity("HUD_Manager");
    hudManager.AddComponent<NativeScriptComponent>().Bind<HUDManager>();

    // Debug
	// FPS Counter
    m_TextFPS = m_ActiveScene->CreateEntity("FPSCounter");
    auto& tc = m_TextFPS.GetComponent<TransformComponent>();
    tc.SetTranslation({ -8.5f, 4.5f, 0.1f });
	tc.SetScale({ 0.50f, 0.50f, 1.0f });

    auto& txtFPS = m_TextFPS.AddComponent<TextComponent>();
    txtFPS.TextString = "FPS: 0";
    txtFPS.Color = { 0.0f, 1.0f, 0.0f, 1.0f };
	txtFPS.Visible = showDebug;

	// VSync Indicator
	m_TextVsync = m_ActiveScene->CreateEntity("VSyncIndicator");
	auto& tcVsync = m_TextVsync.GetComponent<TransformComponent>();
	tcVsync.SetTranslation({ -8.5f, 4.0f, 0.1f });
	tcVsync.SetScale({ 0.50f, 0.50f, 1.0f });

	auto& txtVsync = m_TextVsync.AddComponent<TextComponent>();
	txtVsync.TextString = "VSync";
	txtVsync.Color = { 0.0f, 1.0f, 0.0f, 1.0f };
	txtVsync.Visible = showDebug;

    auto& window = Application::Get().GetWindow();
    m_ActiveScene->OnViewportResize(window.GetWidth(), window.GetHeight());
}

void UILayer::OnDetach()
{
	R2D_PROFILE_FUNCTION();

    if (m_ActiveScene)
        m_ActiveScene->OnRuntimeStop();

	UpgradeDatabase::Shutdown();
}

void UILayer::OnUpdate(Runic2D::Timestep ts)
{
	R2D_PROFILE_FUNCTION();

    RenderCommand::ClearDepth();

    if (m_ActiveScene)
    {
        if (showDebug)
        {
			//FPS Counter
            auto& fpsTxt = m_TextFPS.GetComponent<TextComponent>();
            fpsTxt.TextString = "FPS: " + std::to_string((int)Application::Get().GetAverageFPS());

			//Is VSync
			auto& vsyncTxt = m_TextVsync.GetComponent<TextComponent>();
			auto& window = Application::Get().GetWindow();
			vsyncTxt.TextString = "VSync: " + std::string(window.IsVSync() ? "ON" : "OFF");
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
			// Toggle debug info
            showDebug = !showDebug;

            // fps
            auto& txt = m_TextFPS.GetComponent<TextComponent>();
            txt.Visible = showDebug;

			// vsync
			auto& txtVsync = m_TextVsync.GetComponent<TextComponent>();
			txtVsync.Visible = showDebug;
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
