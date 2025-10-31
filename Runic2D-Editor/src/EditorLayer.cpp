#include "EditorLayer.h"

#include "imgui/imgui.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Runic2D
{
	EditorLayer::EditorLayer() : Layer("EditorLayer"), m_CameraController(1280.0f / 720.0f, true) {

	}

	void EditorLayer::OnAttach()
	{
		R2D_PROFILE_FUNCTION();

		m_Texture = Runic2D::Texture2D::Create("assets/textures/Check.png");
		m_RunicTexture = Runic2D::Texture2D::Create("assets/textures/icon.png");

		m_CameraController.SetZoomLevel(5.0f);

		FrameBufferSpecification fbSpec;
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_FrameBuffer = FrameBuffer::Create(fbSpec);
	}

	void EditorLayer::OnDetach()
	{
		R2D_PROFILE_FUNCTION();

	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		R2D_PROFILE_FUNCTION();

		if (m_ViewportFocused) {
			m_CameraController.OnUpdate(ts);
		}

		//render
		Renderer2D::ResetStats();
		{
			R2D_PROFILE_SCOPE("Renderer Prep")

				m_FrameBuffer->Bind();

			RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
			RenderCommand::Clear();
		}

		{
			R2D_PROFILE_SCOPE("Renderer Draw")

				static float rotation = 0.0f;
			rotation += ts * 50.0f;

			Renderer2D::BeginScene(m_CameraController.GetCamera());

			Renderer2D::DrawRotatedQuad({ 0.5f, -0.5f , -0.1 }, { 0.5f, 1.0f }, glm::radians(rotation), m_SquareColor);
			Renderer2D::DrawRotatedQuad({ 1.0f, 1.0f }, { 0.8f, 0.8f }, glm::radians(-rotation), m_SquareColor);
			Renderer2D::DrawQuad({ -1.0f, 1.0f }, { 1.0f, 1.0f }, m_Texture);
			Renderer2D::DrawQuad({ 0.0f, 0.0f }, { 0.8f, 0.8f }, m_RunicTexture, 1.0f);
			Renderer2D::DrawQuad({ 0.0f, 1.0f }, { 0.8f, 0.8f }, m_RunicTexture, 10.0f);
			Renderer2D::DrawRotatedQuad({ -2.0f, 0.0f, -0.1f }, { 10.0f, 10.0f }, 45.0f, m_Texture, 10.0f, glm::vec4(1.0f, 0.7f, 0.7f, 1.0f));

			Renderer2D::EndScene();

			m_FrameBuffer->Unbind();
		}
	}

	void EditorLayer::OnImGuiRender()
	{
		R2D_PROFILE_FUNCTION();

		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		float avaragefps = Runic2D::Application::Get().GetAverageFPS();
		auto stats = Runic2D::Renderer2D::GetStats();
		ImGui::Begin("Settings");
		ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
		ImGui::Text("Renderer2D Stats");
		ImGui::Text("Avarage FPS: %.2f", avaragefps);
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quad Count: %d", stats.QuadCount);
		ImGui::Text("Vertex Count: %d", stats.GetTotalVertexCount());
		ImGui::Text("Index Count: %d", stats.GetTotalIndexCount());
		ImGui::End();

		//Viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->SetBlockEvents(!m_ViewportFocused || !m_ViewportHovered);
		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		if (m_ViewportSize != *(glm::vec2*)&viewportSize) {
			m_FrameBuffer->Resize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
			m_ViewportSize = { viewportSize.x, viewportSize.y };
			m_CameraController.OnResize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
		}
		uint32_t textureID = m_FrameBuffer->GetColorAttachmentRendererID();
		ImGui::Image((void*)(uintptr_t)textureID, ImVec2{ (float)m_FrameBuffer->GetSpecification().Width, (float)m_FrameBuffer->GetSpecification().Height }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::End();
	}

	void EditorLayer::OnEvent(Runic2D::Event& e)
	{
		m_CameraController.OnEvent(e);
	}
}
