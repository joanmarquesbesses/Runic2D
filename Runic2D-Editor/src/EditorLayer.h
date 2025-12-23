#pragma once

#include <Runic2D.h>
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/SettingsPanel.h"
#include "Panels/ToolbarPanel.h"
#include "Panels/SceneViewportPanel.h"

#include "Runic2D/Renderer/EditorCamera.h"

namespace Runic2D {
	
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;

	private:
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveSceneAs();
		void SaveScene();

		void SerializeScene(Ref<Scene> scene, const std::filesystem::path& path);

		void OnScenePlay();
		void OnSceneStop();

		void OnDuplicateEntity();
	private:
		Ref<FrameBuffer> m_FrameBuffer;
		
		Ref<Scene> m_ActiveScene;
		Ref<Scene> m_EditorScene;
		std::filesystem::path m_EditorScenePath;

		EditorCamera m_EditorCamera;
		Entity m_HoveredEntity;

		SceneState m_SceneState = SceneState::Edit;

		int m_GizmoType = -1;
		int m_GizmoMode = 0;

		bool m_ShowPhysicsColliders = false;

		//Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;
		SettingsPanel m_SettingsPanel;
		ToolbarPanel m_ToolbarPanel;
		SceneViewportPanel m_ViewportPanel;
	};

}

