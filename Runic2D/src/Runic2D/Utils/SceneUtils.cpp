#include "R2Dpch.h"
#include "SceneUtils.h"

#include "Runic2D/Core/App/Application.h"
#include "Runic2D/Core/Input/Input.h"
#include "Runic2D/Scene/Scene.h"
#include "Runic2D/Scene/Components/CoreComponents.h"
#include "Runic2D/Scene/Components/RenderComponents.h"
#include "Runic2D/Scene/Entity.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Runic2D::Utils {

	glm::vec2 SceneUtils::GetMouseWorldPosition(Scene* scene)
	{
		if (!scene) return { 0.0f, 0.0f };

		glm::vec2 input = Input::GetMousePosition();

		auto& window = Application::Get().GetWindow();
		float width = (float)window.GetWidth();
		float height = (float)window.GetHeight();

		glm::vec2 boundsMin = scene->GetViewportBoundsMin();
		glm::vec2 boundsMax = scene->GetViewportBoundsMax();

		float mouseX = input.x;
		float mouseY = input.y;
		float vpWidth = width;
		float vpHeight = height;

		if (boundsMax.x > boundsMin.x && boundsMax.y > boundsMin.y)
		{
			mouseX = input.x - boundsMin.x;
			mouseY = input.y - boundsMin.y;
			vpWidth = boundsMax.x - boundsMin.x;
			vpHeight = boundsMax.y - boundsMin.y;
		}

		if (vpWidth <= 0.0f || vpHeight <= 0.0f)
			return { 0.0f, 0.0f };

		Entity cameraEntity = scene->GetPrimaryCameraEntity();
		if (!cameraEntity) return { 0.0f, 0.0f };

		auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
		auto& camTrans = cameraEntity.GetComponent<TransformComponent>();

		float x = (2.0f * mouseX) / vpWidth - 1.0f;
		float y = 1.0f - (2.0f * mouseY) / vpHeight;
		float z = 1.0f;

		glm::vec3 ndc = { x, y, z };

		glm::mat4 viewProj = camera.GetProjection() * glm::inverse(camTrans.GetTransform());
		glm::mat4 invViewProj = glm::inverse(viewProj);

		glm::vec4 worldPos = invViewProj * glm::vec4(ndc, 1.0f);

		return { worldPos.x, worldPos.y };
	}
}
