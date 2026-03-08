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
    m_DebugInfo = m_ActiveScene->CreateEntity("DebugInfo");
    auto& tc = m_DebugInfo.GetComponent<TransformComponent>();
    tc.SetTranslation({ -8.5f, 4.5f, 0.1f });
	tc.SetScale({ 0.50f, 0.50f, 1.0f });

    auto& txtFPS = m_DebugInfo.AddComponent<TextComponent>();
    txtFPS.Color = { 0.0f, 1.0f, 0.0f, 1.0f };
	txtFPS.Visible = false;

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
        if (m_Context && m_Context->DebugStats.ShowStats)
        {
            m_Context->DebugStats.UIEntities = m_ActiveScene->GetSizeOfAllEntities();

            auto& debugInfo = m_DebugInfo.GetComponent<TextComponent>();
			auto& window = Application::Get().GetWindow();

            int totalEntities = m_Context->DebugStats.GameplayEntities + m_Context->DebugStats.UIEntities;

            std::string debugString =
                "FPS: " + std::to_string((int)Application::Get().GetAverageFPS()) + " | " +
                "VSync: " + std::string(window.IsVSync() ? "ON" : "OFF") + "\n"
                "Entities: " + std::to_string(totalEntities) + " (Gameplay Layer: " + std::to_string(m_Context->DebugStats.GameplayEntities) +
                " | UI Layer: " + std::to_string(m_Context->DebugStats.UIEntities) + ")\n" +
                "Enemies: " + std::to_string(m_Context->DebugStats.TotalEnemies) + "\n" +
                "Bullets: " + std::to_string(m_Context->DebugStats.TotalProjectiles) + "\n" +
                "Particles: " + std::to_string(m_Context->DebugStats.ActiveParticles) + "\n" +
                "DrawCalls: " + std::to_string(m_Context->DebugStats.LastFrameDrawCalls) + "\n" +
                "QuadsDrawn: " + std::to_string(m_Context->DebugStats.LastFrameQuads);

            debugInfo.TextString = debugString;
        }

        m_ActiveScene->OnUpdateRunTime(ts);

        if (m_Context && m_Context->DebugStats.ShowStats) {
            m_Context->DebugStats.LastFrameDrawCalls = Renderer2D::GetStats().DrawCalls;
            m_Context->DebugStats.LastFrameQuads = Renderer2D::GetStats().QuadCount;
        }
    }

    Renderer2D::ResetStats();

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
            if (m_Context) {
                // Toggle debug info
                m_Context->DebugStats.ShowStats = !m_Context->DebugStats.ShowStats;
                bool show = m_Context->DebugStats.ShowStats;

                auto& txt = m_DebugInfo.GetComponent<TextComponent>();
                txt.Visible = show;
            }
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
