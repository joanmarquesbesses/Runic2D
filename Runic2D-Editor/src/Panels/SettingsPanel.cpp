#include "R2Dpch.h"
#include "SettingsPanel.h"
#include "Runic2D/Renderer/Renderer2D.h"
#include "Runic2D/Core/Application.h"
#include "Runic2D/Core/JobSystem.h"
#include "Runic2D/Core/BackgroundTaskSystem.h"
#include "Runic2D/Systems/DebugSystem.h"

#include <imgui/imgui.h>
#include "ImGuizmo.h"

namespace Runic2D {

	void SettingsPanel::OnImGuiRender(Ref<Scene> activeScene, EditorCamera& camera, ContentBrowserPanel& contentBrowser, int& gizmoType, int& gizmoMode, bool& showColliders)
	{
		ImGui::Begin("Settings");

		// --- GIZMOS ---
		if (ImGui::CollapsingHeader("Gizmo", ImGuiTreeNodeFlags_CollapsingHeader)) {
			ImGui::Text("Type");
			if (ImGui::RadioButton("Translate", gizmoType == ImGuizmo::TRANSLATE)) gizmoType = ImGuizmo::TRANSLATE;
			ImGui::SameLine();
			if (ImGui::RadioButton("Rotate", gizmoType == ImGuizmo::ROTATE)) gizmoType = ImGuizmo::ROTATE;
			ImGui::SameLine();
			if (ImGui::RadioButton("Scale", gizmoType == ImGuizmo::SCALE)) gizmoType = ImGuizmo::SCALE;

			ImGui::Separator();
			ImGui::Text("Mode");
			if (ImGui::RadioButton("Local", gizmoMode == ImGuizmo::LOCAL)) gizmoMode = ImGuizmo::LOCAL;
			ImGui::SameLine();
			if (ImGui::RadioButton("World", gizmoMode == ImGuizmo::WORLD)) gizmoMode = ImGuizmo::WORLD;
		}

		// --- EDITOR CAMERA ---
        if (ImGui::CollapsingHeader("Editor Camera", ImGuiTreeNodeFlags_CollapsingHeader))
        {
            EditorCamera::ProjectionType currentProjection = camera.GetProjectionType();

            if (ImGui::RadioButton("Perspective", currentProjection == EditorCamera::ProjectionType::Perspective))
            {
                camera.SetProjectionType(EditorCamera::ProjectionType::Perspective);
				camera.SetRotationLocked(false);
            }

            ImGui::SameLine();

            if (ImGui::RadioButton("Orthographic", currentProjection == EditorCamera::ProjectionType::Orthographic))
            {
                camera.SetProjectionType(EditorCamera::ProjectionType::Orthographic);
                camera.SetRotationLocked(true);
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            bool isLocked = camera.IsRotationLocked();
            if (ImGui::Checkbox("Lock Rotation", &isLocked))
                camera.SetRotationLocked(isLocked);

            if (camera.GetProjectionType() == EditorCamera::ProjectionType::Perspective)
            {
                float fov = camera.GetFOV();
                if (ImGui::DragFloat("Vertical FOV", &fov, 1.0f, 1.0f, 179.0f))
                    camera.SetFOV(fov);

                float nearClip = camera.GetNearClip();
                if (ImGui::DragFloat("Near Clip", &nearClip, 0.1f, 0.001f, 10000.0f))
                    camera.SetNearClip(nearClip);

                float farClip = camera.GetFarClip();
                if (ImGui::DragFloat("Far Clip", &farClip, 10.0f, nearClip, 100000.0f))
                    camera.SetFarClip(farClip);
            }
            else
            {
                float orthoSize = camera.GetOrthographicSize();
                if (ImGui::DragFloat("Size (Zoom)", &orthoSize, 0.1f, 0.1f, 1000.0f))
                    camera.SetOrthographicSize(orthoSize);

                float orthoNear = camera.GetOrthographicNearClip();
                if (ImGui::DragFloat("Near Clip", &orthoNear, 0.1f, -100.0f, 100.0f))
                    camera.SetOrthographicNearClip(orthoNear);

                float orthoFar = camera.GetOrthographicFarClip();
                if (ImGui::DragFloat("Far Clip", &orthoFar, 0.1f, orthoNear, 10000.0f))
                    camera.SetOrthographicFarClip(orthoFar);

                ImGui::TextDisabled("(In Ortho mode, 'Size' controls the Zoom level)");
            }
        }

		// --- CONTENT BROWSER SETTINGS (NOU) ---
		if (ImGui::CollapsingHeader("Content Browser", ImGuiTreeNodeFlags_CollapsingHeader))
		{
			ImGui::SliderFloat("Thumbnail Size", &contentBrowser.GetThumbnailSize(), 16, 512);
			ImGui::SliderFloat("Padding", &contentBrowser.GetPadding(), 0, 32);
		}

		// --- STATS ---
		if (ImGui::CollapsingHeader("Stats", ImGuiTreeNodeFlags_CollapsingHeader)) {
			auto stats = Renderer2D::GetStats();
			ImGui::Text("Draw Calls: %d", stats.DrawCalls);
			ImGui::Text("Quads: %d", stats.QuadCount);
			ImGui::Text("Vertices: %d", stats.GetTotalVertexCount());
			ImGui::Text("Indices: %d", stats.GetTotalIndexCount());
			ImGui::Text("FPS: %.1f", Application::Get().GetAverageFPS());

			ImGui::Separator();
			ImGui::Text("Flush Reasons:");
			static const char* reasons[] = { "Scene End", "Vertex/Index Limit", "Texture Limit", "Primitive Change", "UI ZIndex Change" };
			for (int i = 0; i < (int)Renderer2D::FlushReason::Count; i++)
			{
				if (stats.FlushReasons[i] > 0)
					ImGui::BulletText("%s: %d", reasons[i], stats.FlushReasons[i]);
				else
					ImGui::TextDisabled("  %s: 0", reasons[i]);
			}

			ImGui::Separator();
			ImGui::Text("Scene Stats:");
			auto debugSys = activeScene->GetSystem<DebugSystem>();
			if (debugSys)
			{
				auto stats = debugSys->GetStats(activeScene.get());
				ImGui::BulletText("Total Entities: %d", stats.TotalEntities);
				ImGui::BulletText("Script Updates: %d", stats.ScriptUpdates);
				ImGui::BulletText("Active Particles: %d", stats.ActiveParticles);
			}

			ImGui::Separator();
			bool debugOverlay = debugSys->GetShowStats();
			if (ImGui::Checkbox("Show Debug Overlay", &debugOverlay))
				debugSys->SetShowStats(debugOverlay);

			ImGui::Separator();
			ImGui::Text("Toggle Threading Systems:");
			bool jobEnabled = JobSystem::IsEnabled();
			if (ImGui::Checkbox("Enable Multithreading (JobSystem)", &jobEnabled))
				JobSystem::SetEnabled(jobEnabled);
			bool bgEnabled = BackgroundTaskSystem::IsEnabled();
			if (ImGui::Checkbox("Enable Async Loading (Background)", &bgEnabled))
				BackgroundTaskSystem::SetEnabled(bgEnabled);
		}

        // --- Physics ---
        if (ImGui::CollapsingHeader("Physics", ImGuiTreeNodeFlags_CollapsingHeader))
        {
            ImGui::Checkbox("Show Physics Colliders", &showColliders);
		}

		// --- Profiling ---
		if (ImGui::CollapsingHeader("Profiling", ImGuiTreeNodeFlags_CollapsingHeader))
		{
			static bool s_Profiling = false;
			if (ImGui::Button(s_Profiling ? "Aturar Profiling" : "Iniciar Profiling"))
			{
				if (s_Profiling)
					R2D_PROFILE_END_SESSION();
				else
					R2D_PROFILE_BEGIN_SESSION("Runtime", "Profiling_Result.json");
				s_Profiling = !s_Profiling;
			}
		}

		ImGui::End();
	}
}