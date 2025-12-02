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

		m_Texture = Texture2D::Create("assets/textures/Check.png");
		m_RunicTexture = Texture2D::Create("assets/textures/icon.png");

		m_CameraController.SetZoomLevel(5.0f);

		FrameBufferSpecification fbSpec;
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_FrameBuffer = FrameBuffer::Create(fbSpec);

		m_ActiveScene = CreateRef<Scene>();

		m_SquareEntity = m_ActiveScene->CreateEntity("Quad");
		m_SquareEntity.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.2f, 0.3f, 0.8f, 1.0f });

		m_CameraEntity = m_ActiveScene->CreateEntity("Camera Entity");
		auto& cameraComponent = m_CameraEntity.AddComponent<CameraComponent>();

		m_SecondCameraEntity = m_ActiveScene->CreateEntity("Second Camera Entity");
		auto& secondCameraComponent = m_SecondCameraEntity.AddComponent<CameraComponent>();
		secondCameraComponent.Primary = false;
	}

	void EditorLayer::OnDetach()
	{
		R2D_PROFILE_FUNCTION();

	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		R2D_PROFILE_FUNCTION();

		//Resize
		if(FrameBufferSpecification spec = m_FrameBuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_FrameBuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_CameraController.OnResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		if (m_ViewportFocused) {
			m_CameraController.OnUpdate(ts);
		}

		//render
		Renderer2D::ResetStats();
		m_FrameBuffer->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();

		//update scene
		m_ActiveScene->OnUpdate(ts);

		m_FrameBuffer->Unbind();
		
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
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows, 
				// which we can't undo at the moment without finer window depth/z control.
				//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

				if (ImGui::MenuItem("Exit")) Application::Get().Close();
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}


		float avaragefps = Runic2D::Application::Get().GetAverageFPS();
		auto stats = Runic2D::Renderer2D::GetStats();
		ImGui::Begin("Settings");
		ImGui::Text("Renderer2D Stats");
		ImGui::Text("Avarage FPS: %.2f", avaragefps);
		ImGui::Text("Draw Calls: %d", stats.DrawCalls);
		ImGui::Text("Quad Count: %d", stats.QuadCount);
		ImGui::Text("Vertex Count: %d", stats.GetTotalVertexCount());
		ImGui::Text("Index Count: %d", stats.GetTotalIndexCount());

		if (m_SquareEntity) {
			ImGui::Separator();
			auto& tag = m_SquareEntity.GetComponent<TagComponent>().Tag;
			ImGui::Text("Tag: %s", tag.c_str());

			auto& squareColor = m_SquareEntity.GetComponent<SpriteRendererComponent>().Color;
			ImGui::ColorEdit4("Square Color", glm::value_ptr(squareColor));
			ImGui::Separator();
		}

		ImGui::DragFloat3("Camera Transform",
			glm::value_ptr(m_CameraEntity.GetComponent<TransformComponent>().Transform[3]));

		if (ImGui::Checkbox("Camera A", &m_PrimaryCamera))
		{
			m_CameraEntity.GetComponent<CameraComponent>().Primary = m_PrimaryCamera;
			m_SecondCameraEntity.GetComponent<CameraComponent>().Primary = !m_PrimaryCamera;
		}

		{
			auto& camera = m_SecondCameraEntity.GetComponent<CameraComponent>().Camera;
			float orthoSize = camera.GetOrthographicSize();
			if (ImGui::DragFloat("Camera B Ortho Size", &orthoSize))
			{
				camera.SetOrthographicSize(orthoSize);
			}
		}
		
		ImGui::End();

		//Viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->SetBlockEvents(!m_ViewportFocused || !m_ViewportHovered);

		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportSize.x, viewportSize.y };

		uint32_t textureID = m_FrameBuffer->GetColorAttachmentRendererID();
		ImGui::Image((void*)(uintptr_t)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::End();
	}

	void EditorLayer::OnEvent(Runic2D::Event& e)
	{
		m_CameraController.OnEvent(e);
	}
}
