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

		void OnScenePlay();
		void OnSceneStop();

	private:
		bool OnKeyPressed(KeyPressedEvent& e);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

		void NewScene();
		void OpenScene();
		void OpenScene(const std::filesystem::path& path);
		void SaveSceneAs();
	private:
		Ref<FrameBuffer> m_FrameBuffer;
		Ref<Scene> m_ActiveScene;

		EditorCamera m_EditorCamera;
		Entity m_HoveredEntity;

		int m_GizmoType = -1;
		int m_GizmoMode = 0;

		SceneState m_SceneState = SceneState::Edit;

		//Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
		ContentBrowserPanel m_ContentBrowserPanel;
		SettingsPanel m_SettingsPanel;
		ToolbarPanel m_ToolbarPanel;
		SceneViewportPanel m_ViewportPanel;
	};

}

