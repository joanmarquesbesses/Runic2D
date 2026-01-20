#include "R2Dpch.h"
#include "SceneUtils.h"

#include "Runic2D/Core/Application.h"
#include "Runic2D/Core/Input.h"
#include "Runic2D/Scene/Scene.h"
#include "Runic2D/Scene/Component.h"
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

		Entity cameraEntity = scene->GetPrimaryCameraEntity();

		if (!cameraEntity) return { 0.0f, 0.0f };

		auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
		auto& camTrans = cameraEntity.GetComponent<TransformComponent>();

		float x = (2.0f * input.x) / width - 1.0f;
		float y = 1.0f - (2.0f * input.y) / height;
		float z = 1.0f;

		glm::vec3 ndc = { x, y, z };

		glm::mat4 viewProj = camera.GetProjection() * glm::inverse(camTrans.GetTransform());
		glm::mat4 invViewProj = glm::inverse(viewProj);

		glm::vec4 worldPos = invViewProj * glm::vec4(ndc, 1.0f);

		return { worldPos.x, worldPos.y };
	}
}