#include "R2Dpch.h"
#include "OrthographicCameraController.h"

#include "Runic2D/Core/Input.h"
#include "Runic2D/Core/KeyCodes.h"


namespace Runic2D {

	OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
		: m_AspectRatio(aspectRatio), m_Rotation(rotation),
		m_Bounds({ -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel }),
		m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio* m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel)
		
	{
		
	}

	void OrthographicCameraController::OnUpdate(Timestep ts)
	{
		R2D_PROFILE_FUNCTION();

		if (Input::IsKeyPressed(KeyCode::A)) {
			m_CameraPosition.x -= cos(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts;
			m_CameraPosition.y -= sin(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts;
		}
		else if (Input::IsKeyPressed(KeyCode::D)) {
			m_CameraPosition.x += cos(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts;
			m_CameraPosition.y += sin(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts;
		}

		if (Input::IsKeyPressed(KeyCode::W)) {
			m_CameraPosition.x += -sin(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts;
			m_CameraPosition.y += cos(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts;
		}
		else if (Input::IsKeyPressed(KeyCode::S)) {
			m_CameraPosition.x -= -sin(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts;
			m_CameraPosition.y -= cos(glm::radians(m_CameraRotation)) * m_CameraMoveSpeed * ts;
		}

		if (m_Rotation) 
		{
			if (Input::IsKeyPressed(KeyCode::E))
				m_CameraRotation -= m_CameraRotationSpeed * ts;

			else if (Input::IsKeyPressed(KeyCode::Q))
				m_CameraRotation += m_CameraRotationSpeed * ts;

			if (m_CameraRotation > 180.0f)
				m_CameraRotation -= 360.0f;
			else if (m_CameraRotation <= -180.0f)
				m_CameraRotation += 360.0f;

			m_Camera.SetRotation(m_CameraRotation);
		}

		m_Camera.SetPosition(m_CameraPosition);

		m_CameraMoveSpeed = m_ZoomLevel * 5.0f; // Adjust camera move speed based on zoom level
	}

	void OrthographicCameraController::OnEvent(Event& e)
	{
		R2D_PROFILE_FUNCTION();

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

	void OrthographicCameraController::OnResize(float width, float height)
	{
		R2D_PROFILE_FUNCTION();
		m_AspectRatio = width / height;
		CalculateView();
	}

	void OrthographicCameraController::CalculateView()
	{
		m_Bounds = { -m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel };
		m_Camera.SetProjection(m_Bounds.Left, m_Bounds.Right, m_Bounds.Bottom, m_Bounds.Top);
	}

	bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
	{
		R2D_PROFILE_FUNCTION();

		m_ZoomLevel -= e.GetYOffset() * 0.25f;
		m_ZoomLevel = std::max(m_ZoomLevel, 0.25f); // Prevent zooming out too far
		CalculateView();
		return false;
	}

	bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
	{
		R2D_PROFILE_FUNCTION();

		OnResize((float)e.GetWidth(), (float)e.GetHeight());
		return false;
	}

} // namespace Runic2D