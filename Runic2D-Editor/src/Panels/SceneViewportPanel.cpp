#include "R2Dpch.h"
#include "SceneViewportPanel.h"

#include "Runic2D/Core/Application.h"
#include "Runic2D/Scene/Component.h"
#include "Runic2D/Core/Input.h"
#include "Runic2D/Math/Math.h" 

#include <imgui/imgui.h>
#include "ImGuizmo.h"
#include <glm/gtc/type_ptr.hpp>

namespace Runic2D {

	void SceneViewportPanel::OnImGuiRender(Ref<FrameBuffer> framebuffer, Ref<Scene> scene, EditorCamera& camera, Entity& selectedEntity, int& gizmoType, int& gizmoMode)
	{
		ImGuizmo::BeginFrame();
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGuiWindowClass window_class;
		window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;
		ImGui::SetNextWindowClass(&window_class);
		ImGui::Begin("Viewport");

		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->SetBlockEvents(!m_ViewportFocused && !m_ViewportHovered);

		ImVec2 viewportSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportSize.x, viewportSize.y };

		uint32_t textureID = framebuffer->GetColorAttachmentRendererID();
		ImGui::Image((void*)(uintptr_t)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				std::filesystem::path scenePath = std::filesystem::path("assets") / path;

				if (scenePath.extension().string() == ".r2dscene" && m_OnSceneOpenCallback)
				{
					m_OnSceneOpenCallback(scenePath.string());
				}
			}
			ImGui::EndDragDropTarget();
		}

		// Gizmos
		if (selectedEntity && gizmoType != -1 && !m_IsPlayMode)
		{
			bool isOrthographic = (camera.GetProjectionType() == EditorCamera::ProjectionType::Orthographic);
			ImGuizmo::SetOrthographic(isOrthographic);
			ImGuizmo::SetDrawlist();
			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

			const glm::mat4& cameraProjection = camera.GetProjection();
			glm::mat4 cameraView = camera.GetViewMatrix();

			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 transform = scene->GetWorldTransform(selectedEntity);

			// Snapping
			bool snap = Input::IsKeyPressed(KeyCode::LeftControl);
			float snapValue = 0.5f; 
			if (gizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f;

			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
				(ImGuizmo::OPERATION)gizmoType, (ImGuizmo::MODE)gizmoMode, glm::value_ptr(transform),
				nullptr, snap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				glm::mat4 localTransform = transform;

				if (selectedEntity.HasComponent<RelationshipComponent>())
				{
					auto& rc = selectedEntity.GetComponent<RelationshipComponent>();
					if (rc.Parent != entt::null)
					{
						Entity parent = { rc.Parent, scene.get() };
						glm::mat4 parentWorldTransform = scene->GetWorldTransform(parent);

						if (glm::epsilonNotEqual(glm::determinant(parentWorldTransform), 0.0f, glm::epsilon<float>()))
						{
							localTransform = glm::inverse(parentWorldTransform) * transform;
						}
						else
						{
							ImGui::End();
							ImGui::PopStyleVar();
							return;
						}
					}
				}

				glm::vec3 translation, rotation, scale;
				Math::DecomposeTransform(localTransform, translation, rotation, scale);

				glm::vec3 deltaRotation = rotation - tc.Rotation;
				tc.Translation = translation;
				tc.Rotation += deltaRotation;

				auto checkScale = [](float& s) {
					if (std::isnan(s)) s = 1.0f;
					else if (std::abs(s) < 0.001f) s = 0.001f;
					};

				if (std::abs(scale.x) < 0.001f) scale.x = 0.001f;
				if (std::abs(scale.y) < 0.001f) scale.y = 0.001f;
				if (std::abs(scale.z) < 0.001f) scale.z = 0.001f;

				tc.Scale = scale;

				tc.IsDirty = true;
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}
}
