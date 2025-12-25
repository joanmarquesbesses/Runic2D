#include "EditorLayer.h"

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Runic2D/Scene/SceneSerializer.h"
#include "Runic2D/Utils/PlatformUtils.h"
#include "Runic2D/Project/Project.h"
#include"Runic2D/Assets/ResourceManager.h"

#include "ImGuizmo.h"

namespace Runic2D
{

	EditorLayer::EditorLayer() : Layer("EditorLayer") {

	}

	void EditorLayer::OnAttach()
	{
		R2D_PROFILE_FUNCTION();

		FrameBufferSpecification fbSpec;
		fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth };
		fbSpec.Width = 1280;
		fbSpec.Height = 720;
		m_FrameBuffer = FrameBuffer::Create(fbSpec);

		m_ActiveScene = CreateRef<Scene>();
		m_EditorScene = m_ActiveScene;
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);

		std::filesystem::path projectPath = "Projects/Arkanoid/Arkanoid.r2dproj";

		if (std::filesystem::exists(projectPath))
		{
			OpenProject(projectPath);
		}
		else
		{
			Project::New();
			m_ContentBrowserPanel.ResetToDefault();
		}

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

		m_ContentBrowserPanel.SetOnFileOpenCallback([this](const std::filesystem::path& path)
			{
				if (path.extension().string() == ".r2dscene")
				{
					OpenScene(path);
				}
				else
				{
					R2D_CORE_WARN("File type not supported for opening: {0}", path.string());
				}
			});

		m_ToolbarPanel.SetOnPlayCallback([this]() { OnScenePlay(); });
		m_ToolbarPanel.SetOnStopCallback([this]() { OnSceneStop(); });

		m_ViewportPanel.SetOnSceneOpenCallback([this](const std::string& path) {
			OpenScene(path);
			});

	}

	void EditorLayer::OnDetach()
	{
		R2D_PROFILE_FUNCTION();
		m_EditorScene = nullptr;
		m_ActiveScene = nullptr;
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		R2D_PROFILE_FUNCTION();

		glm::vec2 viewportSize = m_ViewportPanel.GetSize();
		//Resize
		if(FrameBufferSpecification spec = m_FrameBuffer->GetSpecification();
			viewportSize.x > 0.0f && viewportSize.y > 0.0f &&
			(spec.Width != viewportSize.x || spec.Height != viewportSize.y))
		{
			m_FrameBuffer->Resize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
			m_EditorCamera.SetViewportSize(viewportSize.x, viewportSize.y);
			m_ActiveScene->OnViewportResize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
		}

		//render
		Renderer2D::ResetStats();
		m_FrameBuffer->Bind();

		if (m_SceneState == SceneState::Play)
			RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.2f, 1 });
		else
			RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });

		RenderCommand::Clear();

		m_FrameBuffer->ClearAttachment(1, -1);
		//update scene
		switch (m_SceneState)
		{
			case SceneState::Edit:
			{
				m_EditorCamera.OnUpdate(ts);

				m_ActiveScene->OnUpdateEditor(ts, m_EditorCamera);
				break;
			}
			case SceneState::Play:
			{
				m_ActiveScene->OnUpdateRunTime(ts);
				break;
			}
		}

		if (m_ShowPhysicsColliders)
		{
			glm::mat4 viewProj;

			if (m_SceneState == SceneState::Play)
			{
				Entity cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
				if (cameraEntity)
				{
					auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
					auto& tc = cameraEntity.GetComponent<TransformComponent>();
					viewProj = camera.GetProjection() * glm::inverse(tc.GetTransform());
				}
				else
				{
					viewProj = m_EditorCamera.GetViewProjection();
				}
			}
			else
			{
				viewProj = m_EditorCamera.GetViewProjection();
			}

			m_ActiveScene->OnRenderOverlay(viewProj);
		}

		m_FrameBuffer->Unbind();
		
	}

	void EditorLayer::OnImGuiRender()
	{
		R2D_PROFILE_FUNCTION();

		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

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
				if (ImGui::MenuItem("New Project..."))
					NewProject();

				if (ImGui::MenuItem("Open Project..."))
					OpenProject();

				if (ImGui::MenuItem("Save Project"))
					SaveProject();

				ImGui::Separator();

				if(ImGui::MenuItem("New Scene", "Ctrl+N"))
					NewScene();

				if(ImGui::MenuItem("Open Scene...", "Ctrl+O"))
					OpenScene();

				if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
					SaveScene();

				if(ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
					SaveSceneAs();

				ImGui::Separator();

				if (ImGui::MenuItem("Exit")) Application::Get().Close();
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		ImGuizmo::BeginFrame();

		//Panels
		m_SceneHierarchyPanel.OnImGuiRender();
		m_ContentBrowserPanel.OnImGuiRender();
		m_SettingsPanel.OnImGuiRender(m_EditorCamera, m_ContentBrowserPanel, m_GizmoType, m_GizmoMode, m_ShowPhysicsColliders);

		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		m_ViewportPanel.OnImGuiRender(m_FrameBuffer, m_ActiveScene, m_EditorCamera, selectedEntity, m_GizmoType, m_GizmoMode);
		
		m_ToolbarPanel.OnImGuiRender(m_SceneState);

		ImGui::End();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		m_EditorCamera.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(R2D_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(R2D_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		if(e.GetRepeatCount() > 0)
			return false;

		bool control = Input::IsKeyPressed(KeyCode::LeftControl) || Input::IsKeyPressed(KeyCode::RightControl);
		bool shift = Input::IsKeyPressed(KeyCode::LeftShift) || Input::IsKeyPressed(KeyCode::RightShift);

		ImGuiIO& io = ImGui::GetIO();
		if (!io.WantCaptureKeyboard) {
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
				if (control)
				{
					if (shift)
						SaveSceneAs();
					else
						SaveScene();
				}
				break;
			case KeyCode::D:
				if (control)
					OnDuplicateEntity();
				break;
			case KeyCode::Q:
				if (!ImGuizmo::IsUsing()) {
					m_GizmoType = -1;
				}
				break;
			case KeyCode::W:
				if (!ImGuizmo::IsUsing()) {
					m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				}
				break;
			case KeyCode::E:
				if (!ImGuizmo::IsUsing()) {
					m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				}
				break;
			case KeyCode::R:
				if (!ImGuizmo::IsUsing()) {
					m_GizmoType = ImGuizmo::OPERATION::SCALE;
				}
				break;
			default:
				break;
			}
		}	
		return true;
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		if (e.GetMouseButton() == (int)MouseButton::Left)
		{
			// 1. Comprovacions inicials per no fer feina si no cal
			if ((m_GizmoType != -1 && ImGuizmo::IsOver()) || Input::IsKeyPressed(KeyCode::LeftAlt))
				return false;

			auto [mx, my] = ImGui::GetMousePos();
			mx -= m_ViewportPanel.GetBoundsMin().x;
			my -= m_ViewportPanel.GetBoundsMin().y;
			glm::vec2 viewportSize = m_ViewportPanel.GetBoundsMax() - m_ViewportPanel.GetBoundsMin();

			// Invertim Y
			int mouseX = (int)mx;
			int mouseY = (int)viewportSize.y - (int)my;

			if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
			{
				m_FrameBuffer->Bind();
				int pixelData = m_FrameBuffer->ReadPixel(1, mouseX, mouseY);
				m_FrameBuffer->Unbind();

				Entity clickedEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, m_ActiveScene.get());
				m_SceneHierarchyPanel.SetSelectedEntity(clickedEntity);
				m_HoveredEntity = clickedEntity;
			}
		}
		return false;
	}

	void EditorLayer::NewScene()
	{
		if (m_SceneState != SceneState::Edit)
			OnSceneStop();

		m_ActiveScene = CreateRef<Scene>();
		m_EditorScene = m_ActiveScene;

		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportPanel.GetSize().x, (uint32_t)m_ViewportPanel.GetSize().y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
		m_EditorScenePath = std::filesystem::path();
	}

	void EditorLayer::OpenScene()
	{
		std::string filePath = FileDialogs::OpenFile("Runic2D Scene (*.r2dscene)\0*.r2dscene\0");
		if (!filePath.empty())
		{
			OpenScene(filePath);
		}
	}

	void EditorLayer::OpenScene(const std::filesystem::path& path)
	{

		if (m_SceneState != SceneState::Edit) {
			OnSceneStop();
		}

		if (path.extension().string() != ".r2dscene")
		{
			R2D_CORE_WARN("Could not load {0} - not a scene file", path.filename().string());
			return;
		}

		m_EditorScene = nullptr;
		m_ActiveScene = nullptr;

		m_SceneHierarchyPanel.SetContext(nullptr);

		m_HoveredEntity = {};
		m_SceneHierarchyPanel.SetSelectedEntity({});

		Renderer2D::ResetTextureSlots();

		ResourceManager::CleanUpUnused();

		Ref<Scene> newScene = CreateRef<Scene>();
		newScene->OnViewportResize((uint32_t)m_ViewportPanel.GetSize().x, (uint32_t)m_ViewportPanel.GetSize().y);		

		SceneSerializer serializer(newScene);
		if (serializer.Deserialize(path.string()))
		{
			m_EditorScene = newScene;

			m_ActiveScene = m_EditorScene;
			m_EditorScenePath = path;

			m_SceneHierarchyPanel.SetContext(m_EditorScene);
		}
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filePath = FileDialogs::SaveFile("Runic2D Scene (*.r2dscene)\0*.r2dscene\0");
		if (!filePath.empty())
		{
			SerializeScene(m_ActiveScene, filePath);
			m_EditorScenePath = filePath;
		}
	}

	void EditorLayer::SaveScene()
	{
		if (!m_EditorScenePath.empty())
			SerializeScene(m_ActiveScene, m_EditorScenePath);
		else
			SaveSceneAs();
	}

	void EditorLayer::SerializeScene(Ref<Scene> scene, const std::filesystem::path& path)
	{
		SceneSerializer serializer(scene);
		serializer.Serialize(path.string());
	}

	void EditorLayer::OnScenePlay()
	{
		m_SceneState = SceneState::Play;
		m_ActiveScene = Scene::Copy(m_EditorScene);
		m_ViewportPanel.SetPlayMode(true);
		m_ActiveScene->OnRuntimeStart();
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OnSceneStop()
	{
		m_SceneState = SceneState::Edit;
		m_ViewportPanel.SetPlayMode(false);
		m_ActiveScene = m_EditorScene;
		m_ActiveScene->OnRuntimeStop();
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OnDuplicateEntity()
	{
		if (m_SceneState != SceneState::Edit)
			return;

		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selectedEntity)
			m_EditorScene->DuplicateEntity(selectedEntity);
	}

	void EditorLayer::NewProject()
	{
		Project::New();
	}

	void EditorLayer::OpenProject()
	{
		std::string filepath = FileDialogs::OpenFile("Runic2D Project (*.r2dproj)\0*.r2dproj\0");
		if (filepath.empty())
			return;

		OpenProject(filepath);
	}

	void EditorLayer::OpenProject(const std::filesystem::path& path)
	{
		if (Project::Load(path))
		{
			auto startScenePath = Project::GetAssetFileSystemPath(Project::GetActive()->GetConfig().StartScene);
			OpenScene(startScenePath);

			m_ContentBrowserPanel.ResetToDefault();
		}
	}

	void EditorLayer::SaveProject()
	{
		// TODO: Implementar quan tinguem panell de configuració del Projecte
		// (Ex: per canviar l'escena inicial o el nom del joc des de la UI)
		// Project::SaveActive(m_ProjectPath);
	}
}
