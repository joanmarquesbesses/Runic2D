#pragma once
#include "Runic2D/Core/Core.h"
#include "Runic2D/Renderer/FrameBuffer.h"
#include "Runic2D/Scene/Entity.h"
#include <glm/glm.hpp>
#include <functional>

namespace Runic2D {

	class SceneViewportPanel
	{
	public:
		SceneViewportPanel() = default;

		void OnImGuiRender(Ref<FrameBuffer> framebuffer, Ref<Scene> scene, EditorCamera& camera, Entity& selectedEntity, int& gizmoType, int& gizmoMode);

		// Getters per gestionar el resize i el mouse picking desde l'EditorLayer
		glm::vec2 GetSize() const { return m_ViewportSize; }
		glm::vec2 GetBoundsMin() const { return m_ViewportBounds[0]; }
		glm::vec2 GetBoundsMax() const { return m_ViewportBounds[1]; }
		bool IsFocused() const { return m_ViewportFocused; }
		bool IsHovered() const { return m_ViewportHovered; }

		void SetPlayMode(bool playMode) { m_IsPlayMode = playMode; }

		// Callback per quan arrosseguem un fitxer (Drag & Drop)
		void SetOnSceneOpenCallback(const std::function<void(const std::string&)>& callback) { m_OnSceneOpenCallback = callback; }

	private:
		glm::vec2 m_ViewportSize = { 0.0f, 0.0f };
		glm::vec2 m_ViewportBounds[2] = { {0.0f, 0.0f}, {0.0f, 0.0f} };

		bool m_ViewportFocused = false;
		bool m_ViewportHovered = false;

		std::function<void(const std::string&)> m_OnSceneOpenCallback;

		bool m_IsPlayMode = false;
	};
}