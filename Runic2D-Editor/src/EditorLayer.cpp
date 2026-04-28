#include "R2Dpch.h"
#include "EditorLayer.h"

#include <imgui/imgui.h>
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>

#include "Runic2D/Project/Project.h"
#include "Runic2D/Scene/SceneManager.h"
#include "Runic2D/Scene/SceneSerializer.h"
#include "Runic2D/Utils/PlatformUtils.h"

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

		m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);
		m_EditorCamera.SetProjectionType(EditorCamera::ProjectionType::Orthographic);
		m_EditorCamera.SetRotationLocked(true);

		m_ToolbarPanel.SetOnPlayCallback([this]() { OnScenePlay(); });
		m_ToolbarPanel.SetOnStopCallback([this]() { OnSceneStop(); });

		m_ViewportPanel.SetOnSceneOpenCallback([this](const std::string& path) {
			OpenScene(path);
			});

		m_ContentBrowserPanel.SetOnFileOpenCallback([this](const std::filesystem::path& path) {
			if (path.extension() == ".r2dscene")
				OpenScene(path);
			});

		m_EditorScene = CreateRef<Scene>();
		SceneManager::SetActiveScene(m_EditorScene);
		m_SceneHierarchyPanel.SetContext(m_EditorScene);
	}

	void EditorLayer::OnDetach()
	{
		R2D_PROFILE_FUNCTION();

		if (m_SceneState == SceneState::Play)
			SceneManager::StopActiveScene();

		SceneManager::SetSceneChangedCallback(nullptr);

		m_EditorScene = nullptr;
		m_HoveredEntity = {};
		m_SceneHierarchyPanel.SetContext(nullptr);
	}

	void EditorLayer::OnUpdate(Timestep ts)
	{
		R2D_PROFILE_FUNCTION();

		glm::vec2 vpSize = m_ViewportPanel.GetSize();
		if (vpSize.x > 0.f && vpSize.y > 0.f && vpSize != m_LastViewportSize)
		{
			m_LastViewportSize = vpSize;
			m_FrameBuffer->Resize((uint32_t)vpSize.x, (uint32_t)vpSize.y);
			m_EditorCamera.SetViewportSize(vpSize.x, vpSize.y);

			auto scene = SceneManager::GetActiveScene();
			if (scene) scene->OnViewportResize((uint32_t)vpSize.x, (uint32_t)vpSize.y);
		}

		if (m_SceneState == SceneState::Edit && m_ViewportPanel.IsFocused())
			m_EditorCamera.OnUpdate(ts);

		m_FrameBuffer->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1.f });
		RenderCommand::Clear();
		m_FrameBuffer->ClearAttachment(1, -1); // entity ID buffer

		auto scene = SceneManager::GetActiveScene();
		if (scene)
		{
			if (m_SceneState == SceneState::Edit)
			{
				scene->OnUpdateEditor(ts, m_EditorCamera);
				scene->OnRenderUI();
			}
			else // Play
			{
				scene->OnUpdateRunTime(ts);
				scene->OnRenderRuntime();
				scene->OnRenderUI();
			}

			// Overlay de col·lisionadors
			if (m_ShowPhysicsColliders)
			{
				Entity cam = scene->GetPrimaryCameraEntity();
				if (cam)
				{
					auto& camera = cam.GetComponent<CameraComponent>().Camera;
					auto& tc = cam.GetComponent<TransformComponent>();
					glm::mat4 vp = camera.GetProjection() * glm::inverse(tc.GetTransform());
					scene->OnRenderOverlay(vp);
				}
			}

			// Mouse picking (entity ID)
			if (m_SceneState == SceneState::Edit)
			{
				auto [mx, my] = ImGui::GetMousePos();
				mx -= m_ViewportPanel.GetBoundsMin().x;
				my -= m_ViewportPanel.GetBoundsMin().y;
				my = m_LastViewportSize.y - my; // flip Y

				if (mx >= 0 && my >= 0 &&
					mx < m_LastViewportSize.x && my < m_LastViewportSize.y)
				{
					int pixelData = m_FrameBuffer->ReadPixel(1, (int)mx, (int)my);
					m_HoveredEntity = (pixelData == -1)
						? Entity{}
					: Entity{ (entt::entity)pixelData, scene.get() };
				}
			}
		}

		m_FrameBuffer->Unbind();
	}

	void EditorLayer::OnImGuiRender()
	{
		R2D_PROFILE_FUNCTION();

		// - DockSpace
		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);
		windowFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGui::Begin("DockSpace", nullptr, windowFlags);
		ImGui::PopStyleVar(2);

		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
			ImGui::DockSpace(ImGui::GetID("MyDockSpace"));
		style.WindowMinSize.x = minWinSizeX;

		// ---- MENÚ ----
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				// --- Projecte ---
				ImGui::SeparatorText("Project");

				if (ImGui::MenuItem("New Project..."))
					NewProject();

				if (ImGui::MenuItem("Open Project...", "Ctrl+Shift+O"))
					OpenProject();

				if (ImGui::MenuItem("Save Project", "Ctrl+Shift+S", false,
					Project::GetActive() != nullptr))
					SaveProject();

				ImGui::Separator();

				// --- Escena ---
				ImGui::SeparatorText("Scene");

				if (ImGui::MenuItem("New Scene", "Ctrl+N", false,
					Project::GetActive() != nullptr))
					NewScene();

				if (ImGui::MenuItem("Open Scene...", "Ctrl+O", false,
					Project::GetActive() != nullptr))
					OpenScene();

				if (ImGui::MenuItem("Save Scene", "Ctrl+S", false,
					m_EditorScene != nullptr))
					SaveScene();

				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S", false,
					m_EditorScene != nullptr))
					SaveSceneAs();

				ImGui::Separator();

				if (ImGui::MenuItem("Exit"))
					Application::Get().Close();

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		// ---- PANELLS ----
		auto scene = SceneManager::GetActiveScene();

		m_SceneHierarchyPanel.OnImGuiRender();
		m_ContentBrowserPanel.OnImGuiRender();
		m_SettingsPanel.OnImGuiRender(m_EditorCamera, m_ContentBrowserPanel,
			m_GizmoType, m_GizmoMode, m_ShowPhysicsColliders);
		m_ToolbarPanel.OnImGuiRender(m_SceneState);

		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
		m_ViewportPanel.SetPlayMode(m_SceneState == SceneState::Play);
		m_ViewportPanel.OnImGuiRender(m_FrameBuffer, scene, m_EditorCamera,
			selectedEntity, m_GizmoType, m_GizmoMode);

		ImGui::End(); // DockSpace

		Renderer2D::ResetStats();
	}

	void EditorLayer::OnEvent(Event& e)
	{
		if (m_SceneState == SceneState::Edit)
			m_EditorCamera.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(R2D_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(R2D_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		if (e.GetRepeatCount() > 0) return false;

		bool ctrl = Input::IsKeyPressed(KeyCode::LeftControl) ||
			Input::IsKeyPressed(KeyCode::RightControl);
		bool shift = Input::IsKeyPressed(KeyCode::LeftShift) ||
			Input::IsKeyPressed(KeyCode::RightShift);

		switch (e.GetKeyCode())
		{
			// Projecte
		case KeyCode::O:
			if (ctrl && shift) { OpenProject(); return true; }
			if (ctrl) { OpenScene();   return true; }
			break;
		case KeyCode::S:
			if (ctrl && shift) { SaveProject(); return true; }
			if (ctrl) { SaveScene();   return true; }
			break;
		case KeyCode::N:
			if (ctrl) { NewScene(); return true; }
			break;
		case KeyCode::D:
			if (ctrl) { OnDuplicateEntity(); return true; }
			break;

			// Gizmos
		case KeyCode::Q: m_GizmoType = -1;                   break;
		case KeyCode::W: m_GizmoType = ImGuizmo::TRANSLATE;  break;
		case KeyCode::E: m_GizmoType = ImGuizmo::ROTATE;     break;
		case KeyCode::R: m_GizmoType = ImGuizmo::SCALE;      break;
		}
		return false;
	}

	bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		if (e.GetMouseButton() != (int)MouseButton::Left)   return false;
		if (!m_ViewportPanel.IsHovered())                   return false;
		if (ImGuizmo::IsOver())                             return false;
		if (Input::IsKeyPressed(KeyCode::LeftAlt))          return false;
		if (m_SceneState != SceneState::Edit)               return false;

		m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
		return false;
	}

	void EditorLayer::NewScene()
	{
		if (m_SceneState == SceneState::Play)
			OnSceneStop();

		m_EditorScene = CreateRef<Scene>();
		m_EditorScenePath = "";
		SceneManager::SetActiveScene(m_EditorScene);
		m_SceneHierarchyPanel.SetContext(m_EditorScene);
	}

	void EditorLayer::OpenScene()
	{
		std::string initialDir;
		if (Project::GetActive())
		{
			std::filesystem::path sceneDir = Project::GetAssetDirectory() / "scenes";
			std::filesystem::create_directories(sceneDir);
			initialDir = sceneDir.string();
		}

		std::string filepath = FileDialogs::OpenFile(
			"Runic2D Scene (*.r2dscene)\0*.r2dscene\0",
			initialDir.empty() ? nullptr : initialDir.c_str()
		);

		if (!filepath.empty())
			OpenScene(filepath);
	}

	void EditorLayer::OpenScene(const std::filesystem::path& absolutePath)
	{
		if (absolutePath.extension() != ".r2dscene")
		{
			R2D_CORE_WARN("EditorLayer: el fitxer '{0}' no és una escena vàlida.",
				absolutePath.string());
			return;
		}

		if (m_SceneState == SceneState::Play)
			OnSceneStop();

		auto newScene = CreateRef<Scene>();
		SceneSerializer serializer(newScene);
		if (!serializer.Deserialize(absolutePath.string()))
		{
			R2D_CORE_ERROR("EditorLayer: no s'ha pogut carregar l'escena '{0}'",
				absolutePath.string());
			return;
		}

		m_EditorScene = newScene;
		m_EditorScenePath = absolutePath;

		auto viewportSize = m_ViewportPanel.GetSize();
		if (viewportSize.x > 0.0f && viewportSize.y > 0.0f)
		{
			m_EditorScene->OnViewportResize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
		}

		SceneManager::SetActiveScene(m_EditorScene);
		m_SceneHierarchyPanel.SetContext(m_EditorScene);

		R2D_CORE_INFO("EditorLayer: Escena oberta: '{0}'", absolutePath.string());
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string initialDir;
		if (Project::GetActive())
		{
			std::filesystem::path sceneDir = Project::GetAssetDirectory() / "scenes";
			std::filesystem::create_directories(sceneDir);
			initialDir = sceneDir.string();
		}

		std::string filepath = FileDialogs::SaveFile(
			"Runic2D Scene (*.r2dscene)\0*.r2dscene\0",
			initialDir.empty() ? nullptr : initialDir.c_str()
		);

		if (filepath.empty()) return;

		std::filesystem::path path = filepath;
		if (path.extension() != ".r2dscene")
			path += ".r2dscene";

		m_EditorScenePath = path;
		SerializeScene(m_EditorScene, path);
	}

	void EditorLayer::SaveScene()
	{
		if (!m_EditorScenePath.empty())
			SerializeScene(m_EditorScene, m_EditorScenePath);
		else
			SaveSceneAs();
	}

	void EditorLayer::SerializeScene(Ref<Scene> scene, const std::filesystem::path& path)
	{
		SceneSerializer serializer(scene);
		serializer.Serialize(path.string());
		R2D_CORE_INFO("EditorLayer: Escena guardada a '{0}'", path.string());
	}

	void EditorLayer::OnScenePlay()
	{
		R2D_CORE_ASSERT(m_EditorScene, "No hi ha escena d'editor!");

		m_SceneState = SceneState::Play;
		m_ViewportPanel.SetPlayMode(true);

		// Còpia de l'escena perquè el runtime no modifiqui l'original
		Ref<Scene> runtimeScene = Scene::Copy(m_EditorScene);

		// Re-binding de scripts (necessari a la còpia)
		auto view = runtimeScene->GetAllEntitiesWith<NativeScriptComponent>();
		for (auto e : view)
		{
			Entity entity = { e, runtimeScene.get() };
			auto& nsc = entity.GetComponent<NativeScriptComponent>();
			if (!nsc.ClassName.empty())
				ScriptEngine::BindScript(nsc.ClassName, entity);
		}

		SceneManager::SetActiveScene(runtimeScene);
		m_SceneHierarchyPanel.SetContext(runtimeScene);
		SceneManager::GetActiveScene()->OnRuntimeStart();

		auto& window = Application::Get().GetWindow();
		SceneManager::GetActiveScene()->OnViewportResize(
			(uint32_t)m_LastViewportSize.x, (uint32_t)m_LastViewportSize.y);

		R2D_CORE_INFO("EditorLayer: Play mode iniciat.");
	}

	void EditorLayer::OnSceneStop()
	{
		m_SceneState = SceneState::Edit;
		m_ViewportPanel.SetPlayMode(false);

		// Parar el runtime (física, scripts, etc.)
		SceneManager::StopActiveScene();

		// Restaurar l'escena d'editor (sense runtime)
		SceneManager::SetActiveScene(m_EditorScene);
		m_SceneHierarchyPanel.SetContext(m_EditorScene);

		R2D_CORE_INFO("EditorLayer: Stop mode. Escena d'editor restaurada.");
	}

	void EditorLayer::OnDuplicateEntity()
	{
		if (m_SceneState != SceneState::Edit) return;

		Entity selected = m_SceneHierarchyPanel.GetSelectedEntity();
		if (selected)
			m_EditorScene->DuplicateEntity(selected);
	}

	void EditorLayer::NewProject()
	{
		if (m_SceneState != SceneState::Edit)
			OnSceneStop();

		CleanupCurrentProject();

		// 1. Assegurar que la carpeta "Projects" existeix
		std::filesystem::path projectsRoot =
			std::filesystem::current_path() / "Projects";
		std::filesystem::create_directories(projectsRoot);

		// 2. Obrir SaveFile apuntant a "Projects/"
		std::string projectsRootStr = projectsRoot.string();
		std::string rawPath = FileDialogs::SaveFile(
			"Runic2D Project (*.r2dproj)\0*.r2dproj\0",
			projectsRootStr.c_str()   // <-- initialDir: obre el diàleg a Projects/
		);

		if (rawPath.empty()) return; // l'usuari ha cancel·lat

		// 3. Extreure el nom del projecte a partir del fitxer escollit
		//    Ex: rawPath = "C:/.../Projects/MyGame.r2dproj"  → name = "MyGame"
		std::filesystem::path chosenPath = rawPath;

		// Assegurar extensió (per si l'usuari no l'ha posat)
		if (chosenPath.extension() != ".r2dproj")
			chosenPath += ".r2dproj";

		std::string projectName = chosenPath.stem().string(); // "MyGame"

		// 4. Construir la ruta REAL: Projects/MyGame/MyGame.r2dproj
		//    (subcarpeta amb el mateix nom del projecte)
		std::filesystem::path projectFolder = chosenPath.parent_path() / projectName;
		std::filesystem::path finalProjFile = projectFolder / chosenPath.filename();
		std::filesystem::path assetsFolder = projectFolder / "Assets";
		std::filesystem::path scenesFolder = assetsFolder / "scenes";

		// 5. Crear tota l'estructura de carpetes
		std::filesystem::create_directories(projectFolder);
		std::filesystem::create_directories(assetsFolder);
		std::filesystem::create_directories(scenesFolder);

		R2D_CORE_INFO("EditorLayer: Creant projecte '{}' a '{}'",
			projectName, projectFolder.string());

		// 6. Configurar el ProjectConfig
		Project::New();
		auto& config = Project::GetActive()->GetConfigMut();
		config.Name = projectName;
		config.AssetDirectory = "Assets";
		config.StartScene = "scenes/MainScene.r2dscene";
		config.ScriptModulePath = projectName + ".dll";

		// 7. Guardar el .r2dproj a la subcarpeta del projecte
		if (!Project::Save(finalProjFile))
		{
			R2D_CORE_ERROR("EditorLayer: Error guardant el projecte a '{}'",
				finalProjFile.string());
			return;
		}

		// 8. Crear una escena inicial buida i guardar-la
		OnProjectLoaded();

		UpdateWindowTitle();
	}

	void EditorLayer::OpenProject()
	{
		std::filesystem::path projectsDir = std::filesystem::current_path() / "Projects";
		std::filesystem::create_directories(projectsDir);

		std::string filepath = FileDialogs::OpenFile(
			"Runic2D Project (*.r2dproj)\0*.r2dproj\0",
			projectsDir.string().c_str()
		);

		if (!filepath.empty())
			OpenProject(filepath);
	}

	void EditorLayer::OpenProject(const std::filesystem::path& path)
	{
		if (!std::filesystem::exists(path))
		{
			R2D_CORE_ERROR("El fitxer de projecte no existeix: {0}", path.string());
			return;
		}

		// 2. Ara sí, desmuntem l'antic (Això evita els crashes de memòria)
		CleanupCurrentProject();

		// 3. Carreguem el nou
		if (Project::Load(path))
		{
			// 4. Connectem la nova DLL i carreguem la UI/Escena
			Project::LoadRuntimeLibrary();
			OnProjectLoaded();
		}
		else
		{
			R2D_CORE_ERROR("Ha fallat la càrrega del projecte a: {0}", path.string());
		}
	}

	void EditorLayer::SaveProject()
	{
		if (!Project::GetActive()) return;

		auto projectDir = Project::GetProjectDirectory();
		auto projectFile = projectDir / (Project::GetConfig().Name + ".r2dproj");
		Project::Save(projectFile);
	}

	void EditorLayer::CleanupCurrentProject()
	{
		if (m_SceneState == SceneState::Play)
			OnSceneStop();

		m_HoveredEntity = {};
		m_SceneHierarchyPanel.SetContext(nullptr);
		m_EditorScene = nullptr;
		SceneManager::SetActiveScene(nullptr);
		Runic2D::ComponentRegistry::Clear();

		if (Project::GetActive())
			Project::UnloadRuntimeLibrary();
	}

	void EditorLayer::OnProjectLoaded()
	{
		if (!Project::GetActive())
		{
			R2D_CORE_ERROR("S'ha intentat cridar OnProjectLoaded però no hi ha cap projecte actiu!");
			return;
		}

		m_ContentBrowserPanel.SetRootDirectory(Project::GetAssetDirectory());

		auto startScene = Project::GetConfig().StartScene;
		if (!startScene.empty())
		{
			auto scenePath = Project::GetAssetFileSystemPath(startScene);

			if (std::filesystem::exists(scenePath))
			{
				OpenScene(scenePath);
			}
			else
			{
				R2D_CORE_WARN("L'escena inicial no existeix. Creant una de nova a: {0}", scenePath.string());

				NewScene();
				std::filesystem::create_directories(scenePath.parent_path());
				SerializeScene(m_EditorScene, scenePath);
				m_EditorScenePath = scenePath;
			}
		}
		else
		{
			NewScene();
		}

		UpdateWindowTitle();
		R2D_CORE_INFO("EditorLayer: Projecte '{}' carregat.",
			Project::GetConfig().Name);
	}

	void EditorLayer::UpdateWindowTitle()
	{
		std::string title = "Runic2D Editor";
		if (Project::GetActive())
			title += " — " + Project::GetConfig().Name;
		Application::Get().GetWindow().SetTitle(title); // si tens aquest mètode
	}
}
