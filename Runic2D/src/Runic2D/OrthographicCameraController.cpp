#include "R2Dpch.h"
#include "OrthographicCameraController.h"

#include "Runic2D/Input.h"
#include "Runic2D/KeyCodes.h"


namespace Runic2D {

	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
		: m_Camera(-aspectRatio * m_ZoomLevel, aspectRatio* m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel),
		m_AspectRatio(aspectRatio), m_Rotation(rotation)
	{
		
	}

	void OrthographicCameraController::OnUpdate(Timestep ts)
	{
		if (Input::IsKeyPressed(R2D_KEY_A))
			m_CameraPosition.x -= m_CameraMoveSpeed * ts;

		else if (Input::IsKeyPressed(R2D_KEY_D))
			m_CameraPosition.x += m_CameraMoveSpeed * ts;

		else if (Input::IsKeyPressed(R2D_KEY_W))
			m_CameraPosition.y += m_CameraMoveSpeed * ts;

		else if (Input::IsKeyPressed(R2D_KEY_S))
			m_CameraPosition.y -= m_CameraMoveSpeed * ts;

		if (m_Rotation) 
		{
			if (Input::IsKeyPressed(R2D_KEY_E))
				m_CameraRotation -= m_CameraRotationSpeed * ts;

			else if (Input::IsKeyPressed(R2D_KEY_Q))
				m_CameraRotation += m_CameraRotationSpeed * ts;

			m_Camera.SetRotation(m_CameraRotation);
		}

		m_Camera.SetPosition(m_CameraPosition);

		m_CameraMoveSpeed = m_ZoomLevel * 5.0f; // Adjust camera move speed based on zoom level
	}

	void OrthographicCameraController::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseScrolledEvent>(RUNIC2D_BIND_EVENT_FN(OrthographicCameraController::OnMouseScrolled));
		dispatcher.Dispatch<WindowResizeEvent>(RUNIC2D_BIND_EVENT_FN(OrthographicCameraController::OnWindowResized));

		//if (e.IsInCategory(EventCategoryMouse | EventCategoryInput))
		//	return;

		//if (e.IsInCategory(EventCategoryMouse))
		//	return;

		//if (e.IsInCategory(EventCategoryKeyboard | EventCategoryInput))
		//	return;
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{

		m_ZoomLevel -= e.GetYOffset() * 0.25f;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f); // Prevent zooming out too far
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);

		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		m_AspectRatio = (float)e.GetWidth() / (float)e.GetHeight();
		m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
		return false;
	}

} // namespace Runic2D