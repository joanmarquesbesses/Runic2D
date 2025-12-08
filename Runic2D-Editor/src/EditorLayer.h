#pragma once

#include <Runic2D.h>
#include "Panels/SceneHierarchyPanel.h"

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
		void SaveSceneAs();
	private:
		//temp
		Ref<VertexArray> m_SquareVA;
		Ref<Shader> m_FlatColorShader;

		Ref<Texture2D> m_Texture, m_RunicTexture;
		Ref<FrameBuffer> m_FrameBuffer;

		Ref<Scene> m_ActiveScene;
		Entity m_SquareEntity;
		Entity m_SecondSquareEntity;
		Entity m_CameraEntity;
		Entity m_SecondCameraEntity;

		EditorCamera m_EditorCamera;

		bool m_ViewportFocused = false, m_ViewportHovered = false;
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };

		glm::vec4 m_SquareColor = { 0.2f, 0.3f, 0.8f, 1.0f };

		int m_GizmoType = -1;
		int m_GizmoMode = 0;

		//Panels
		SceneHierarchyPanel m_SceneHierarchyPanel;
	};

}

