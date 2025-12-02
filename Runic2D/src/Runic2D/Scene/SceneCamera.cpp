#include "R2Dpch.h"
#include "SceneCamera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Runic2D
{
	SceneCamera::SceneCamera()
	{
		RecalculateProjection();
	}

	void Runic2D::SceneCamera::SetOrthographic(float size, float nearClip, float farClip)
	{
		m_OrthographicSize = size;
		m_OrthographicNear = nearClip;
		m_OrthographicFar = farClip;

		RecalculateProjection();
	}

	void SceneCamera::SetViewportSize(uint32_t width, uint32_t height)
	{
		m_AspectRatio = (float)width / (float)height;
		RecalculateProjection();
	}

	void SceneCamera::RecalculateProjection()
	{
		float left = -m_OrthographicSize * 0.5f * m_AspectRatio;
		float right = m_OrthographicSize * 0.5f * m_AspectRatio;
		float bottom = -m_OrthographicSize * 0.5f;
		float top = m_OrthographicSize * 0.5f;

		m_Projection = glm::ortho(left, right, bottom, top,
			m_OrthographicNear, m_OrthographicFar);
	}

}

