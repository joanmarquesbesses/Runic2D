#pragma once

#include "Runic2D/Renderer/OrthographicCamera.h"
#include "Runic2D/Core/Timestep.h"

#include "Runic2D/Events/ApplicationEvent.h"
#include "Runic2D/Events/MouseEvent.h"

namespace Runic2D {

	struct OrthographicCameraBounds
	{
		float Left, Right;
		float Bottom, Top;

		float GetWidth() const { return Right - Left; }
		float GetHeight() const { return Top - Bottom; }
	};

	class OrthographicCameraController
	{
	public:
		OrthographicCameraController(float aspectRatio, bool rotation = false);

		void OnUpdate(Timestep ts);
		void OnEvent(Event& e);

		float GetZoomLevel() const { return m_ZoomLevel; }
		void SetZoomLevel(float level) { m_ZoomLevel = level; CalculateView(); }

		const OrthographicCamera& GetCamera() const { return m_Camera; }
		OrthographicCamera& GetCamera() { return m_Camera; }

		const OrthographicCameraBounds& GetBounds() const { return m_Bounds; }
	private:
		void CalculateView();

		bool OnMouseScrolled(MouseScrolledEvent& e);
		bool OnWindowResized(WindowResizeEvent& e);

	private:
		float m_AspectRatio;
		float m_ZoomLevel = 1.0f;
		bool m_Rotation;

		OrthographicCameraBounds m_Bounds;
		OrthographicCamera m_Camera;

		glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };
		float m_CameraRotation = 0.0f; // In degrees
		float m_CameraMoveSpeed = 5.0f;
		float m_CameraRotationSpeed = 180.0f;
	};

} // namespace Runic2D