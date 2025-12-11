#include "EditorLayer.h"

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Runic2D/Scene/SceneSerializer.h"
#include "Runic2D/Utils/PlatformUtils.h"

#include "Runic2D/Math/Math.h"

#include "ImGuizmo.h"

namespace Runic2D
{
	EditorLayer::EditorLayer() : Layer("EditorLayer") {

	}

	void EditorLayer::OnAttach()
	{
		R2D_PROFILE_FUNCTION();

		m_Texture = Texture2D::Create("assets/textures/Check.png");
		m_RunicTexture = Texture2D::Create("assets/textures/icon.png");

		FrameBufferSpecification fbSpec;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::Depth };
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_FrameBuffer = FrameBuffer::Create(fbSpec);

		m_ActiveScene = CreateRef<Scene>();

		m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);

#if 0
		m_SquareEntity = m_ActiveScene->CreateEntity("Quad");
		m_SquareEntity.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.2f, 0.3f, 0.8f, 1.0f });

		m_SecondSquareEntity = m_ActiveScene->CreateEntity("Second Quad");
		auto& sr = m_SecondSquareEntity.AddComponent<SpriteRendererComponent>();
		sr.Color = glm::vec4{ 0.8f, 0.3f, 0.2f, 1.0f };

		m_CameraEntity = m_ActiveScene->CreateEntity("Camera Entity");
		auto& cameraComponent = m_CameraEntity.AddComponent<CameraComponent>();

		m_SecondCameraEntity = m_ActiveScene->CreateEntity("Second Camera Entity");
		auto& secondCameraComponent = m_SecondCameraEntity.AddComponent<CameraComponent>();
		secondCameraComponent.Primary = false;

		class CameraControler : public ScriptableEntity
		{
		public:

			virtual void OnCreate() override {
			}

			virtual void OnDestroy() override {

			}

			virtual void OnUpdate(Timestep ts) override {
				auto& tc = GetComponent<TransformComponent>();
				float speed = 5.0f;

				// Modifiquem la Translation directament
				if (Input::IsKeyPressed(KeyCode::A))
					tc.Translation.x -= speed * ts;
				if (Input::IsKeyPressed(KeyCode::D))
					tc.Translation.x += speed * ts;
				if (Input::IsKeyPressed(KeyCode::W))
					tc.Translation.y += speed * ts;
				if (Input::IsKeyPressed(KeyCode::S))
					tc.Translation.y -= speed * ts;

				tc.IsDirty = true;
			}
		};

		m_SecondCameraEntity.AddComponent<NativeScriptComponent>().Bind<CameraControler>();
#endif

		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
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
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}

		m_EditorCamera.OnUpdate(ts);

		//render
		Renderer2D::ResetStats();
		m_FrameBuffer->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();
		//update scene
		m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);

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
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = 32.0f;

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows, 
				// which we can't undo at the moment without finer window depth/z control.
				//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);
				if(ImGui::MenuItem("New", "Ctrl+N"))
					NewScene();

				ImGui::Separator();

				if(ImGui::MenuItem("Open...", "Ctrl+O"))
					OpenScene();

				ImGui::Separator();

				if(ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
					SaveSceneAs();

				ImGui::Separator();

				if (ImGui::MenuItem("Exit")) Application::Get().Close();
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		//Scene Hierarchy Panel
		m_SceneHierarchyPanel.OnImGuiRender();

		ImGui::Begin("Settings");
		
		// Gizmo Type
		if (ImGui::CollapsingHeader("Gizmo", ImGuiTreeNodeFlags_CollapsingHeader)){

			ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
			ImGui::Text("Gizmo Type");
			ImGui::PopFont();
			ImGui::Separator();

			if (ImGui::RadioButton("Translate", m_GizmoType == ImGuizmo::TRANSLATE))
				m_GizmoType = ImGuizmo::TRANSLATE;

			ImGui::SameLine();

			if (ImGui::RadioButton("Rotate", m_GizmoType == ImGuizmo::ROTATE))
				m_GizmoType = ImGuizmo::ROTATE;

			ImGui::SameLine();

			if (ImGui::RadioButton("Scale", m_GizmoType == ImGuizmo::SCALE))
				m_GizmoType = ImGuizmo::SCALE;

			ImGui::Separator();

			// Gizmo Mode
			ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
			ImGui::Text("Gizmo Mode");
			ImGui::PopFont();
			ImGui::Separator();

			if (ImGui::RadioButton("Local", m_GizmoMode == ImGuizmo::LOCAL))
				m_GizmoMode = ImGuizmo::LOCAL;

			ImGui::SameLine();

			if (ImGui::RadioButton("World", m_GizmoMode == ImGuizmo::WORLD))
				m_GizmoMode = ImGuizmo::WORLD;
		}
		
		// Editor Camera Settings
		if (ImGui::CollapsingHeader("Editor Camera", ImGuiTreeNodeFlags_CollapsingHeader))
		{
			bool isLocked = m_EditorCamera.IsRotationLocked();
			if (ImGui::Checkbox("Lock Rotation (2D Mode)", &isLocked))
			{
				m_EditorCamera.SetRotationLocked(isLocked);
			}

			// Ajuda visual (Tooltip)
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Blocks camera rotation to work in 2D");

			ImGui::Separator();

			float fov = m_EditorCamera.GetFOV();
			if (ImGui::DragFloat("FOV", &fov, 1.0f, 1.0f, 180.0f))
			{
				m_EditorCamera.SetFOV(fov);
			}

			// 3. CLIPPING PLANES (Near / Far)
			float nearClip = m_EditorCamera.GetNearClip();
			float farClip = m_EditorCamera.GetFarClip();

			bool changed = false;
			if (ImGui::DragFloat("Near Clip", &nearClip, 0.1f, 0.001f, 10000.0f)) {
				m_EditorCamera.SetNearClip(nearClip);
			}

			if (ImGui::DragFloat("Far Clip", &farClip, 10.0f, nearClip, 100000.0f)) {
				m_EditorCamera.SetFarClip(farClip);
			}

			ImGui::Separator();

			float distance = m_EditorCamera.GetDistance();
			if (ImGui::DragFloat("Distance", &distance, 0.2f))
			{
				m_EditorCamera.SetDistance(distance); 
			}
		}

		if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_CollapsingHeader)) {
			float avaragefps = Runic2D::Application::Get().GetAverageFPS();
			auto stats = Runic2D::Renderer2D::GetStats();

			ImGui::Text("Renderer2D Stats");
			ImGui::Text("Avarage FPS: %.2f", avaragefps);
			ImGui::Text("Draw Calls: %d", stats.DrawCalls);
			ImGui::Text("Quad Count: %d", stats.QuadCount);
			ImGui::Text("Vertex Count: %d", stats.GetTotalVertexCount());
			ImGui::Text("Index Count: %d", stats.GetTotalIndexCount());
		}

		ImGui::End();

		//Viewport
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->SetBlockEvents(!m_ViewportFocused && !m_ViewportHovered);

		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportSize.x, viewportSize.y };

		uint32_t textureID = m_FrameBuffer->GetColorAttachmentRendererID();
		ImGui::Image((void*)(uintptr_t)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		// Gizmos
		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selectedEntity && m_GizmoType != -1)
		{
			ImGuizmo::SetDrawlist();

			float windowWidth = (float)ImGui::GetWindowWidth();
			float windowHeight = (float)ImGui::GetWindowHeight();
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

			// Camera
			// Run time camera from entity with camera component
			//auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
			//if (!cameraEntity) return;
			//const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
			//const glm::mat4& cameraProjection = camera.GetProjection();
			//ImGuizmo::SetOrthographic((int)camera.GetProjectionType());
			//glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());

			// Editor camera
			const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
			glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

			// Entity transform
			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 transform = tc.GetTransform();

			// Snapping
			bool snap = Input::IsKeyPressed(KeyCode::LeftControl);
			float snapValue = 0.5f; // Snap to 0.5m for translation/scale
			// Snap to 45 degrees for rotation
			if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f;

			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				(ImGuizmo::OPERATION)m_GizmoType, (ImGuizmo::MODE)m_GizmoMode, glm::value_ptr(transform),
				nullptr, snap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				glm::vec3 translation, rotation, scale;
				Math::DecomposeTransform(transform, translation, rotation, scale);

				glm::vec3 deltaRotation = rotation - tc.Rotation;
				tc.Translation = translation;
				tc.Rotation += deltaRotation;
				tc.Scale = scale;

				tc.IsDirty = true;
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_EditorCamera.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(R2D_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		if(e.GetRepeatCount() > 0)
			return false;

		bool control = Input::IsKeyPressed(KeyCode::LeftControl) || Input::IsKeyPressed(KeyCode::RightControl);
		bool shift = Input::IsKeyPressed(KeyCode::LeftShift) || Input::IsKeyPressed(KeyCode::RightShift);

		switch (e.GetKeyCode())
		{
		case KeyCode::N:
			if (control)
				NewScene();

			break;
		case KeyCode::O:
			if (control)
				OpenScene();

			break;
		case KeyCode::S:
			if (control && shift)
				SaveSceneAs();
		case KeyCode::Q:
			m_GizmoType = -1;
			break;
		case KeyCode::W:
			m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			break;
		case KeyCode::E:
			m_GizmoType = ImGuizmo::OPERATION::ROTATE;
			break;
		case KeyCode::R:
			m_GizmoType = ImGuizmo::OPERATION::SCALE;
			break;
		default:
			break;
		}
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		return false;
	}

	void EditorLayer::NewScene()
	{
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OpenScene()
	{
		std::string filePath = FileDialogs::OpenFile("Runic2D Scene (*.r2dscene)\0*.r2dscene\0");
		if (!filePath.empty())
		{
			m_ActiveScene = CreateRef<Scene>();
			m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_SceneHierarchyPanel.SetContext(m_ActiveScene);

			SceneSerializer serializer(m_ActiveScene);
			serializer.Deserialize(filePath);
		}
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filePath = FileDialogs::SaveFile("Runic2D Scene (*.r2dscene)\0*.r2dscene\0");
		if (!filePath.empty())
		{
			SceneSerializer serializer(m_ActiveScene);
			serializer.Serialize(filePath);
		}
	}
}
