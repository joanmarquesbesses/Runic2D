#include "R2Dpch.h"
#include "ToolbarPanel.h"

#include "Runic2D/Assets/ResourceManager.h"
#include <imgui/imgui.h>

namespace Runic2D {

	ToolbarPanel::ToolbarPanel()
	{
		m_IconPlay = ResourceManager::Get<Texture2D>("Resources/Icons/ToolPanel/play.png");
		m_IconStop = ResourceManager::Get<Texture2D>("Resources/Icons/ToolPanel/stop.png");
	}

	void ToolbarPanel::OnImGuiRender(SceneState state)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));

		auto& colors = ImGui::GetStyle().Colors;
		const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
		const auto& buttonActive = colors[ImGuiCol_ButtonActive];

		ImGuiWindowClass window_class;
		window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;
		ImGui::SetNextWindowClass(&window_class);

		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		float size = ImGui::GetWindowHeight() - 4.0f;
		// Centrat horitzontal
		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));

		Ref<Texture2D> icon = (state == SceneState::Edit) ? m_IconPlay : m_IconStop;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));

		// Botó invisible per capturar events
		if (ImGui::InvisibleButton("##toolbar_play_stop", ImVec2(size, size)))
		{
			if (state == SceneState::Edit)
			{
				if (m_OnPlayCallback) m_OnPlayCallback();
			}
			else if (state == SceneState::Play)
			{
				if (m_OnStopCallback) m_OnStopCallback();
			}
		}
		ImGui::PopStyleVar();

		// Dibuixat manual
		bool isHovered = ImGui::IsItemHovered();
		bool isActive = ImGui::IsItemActive();
		ImVec2 pMin = ImGui::GetItemRectMin();
		ImVec2 pMax = ImGui::GetItemRectMax();
		ImVec2 center = { pMin.x + size * 0.5f, pMin.y + size * 0.5f };

		auto* drawList = ImGui::GetWindowDrawList();

		if (isActive)
			drawList->AddCircleFilled(center, size * 0.5f, ImGui::GetColorU32(buttonActive));
		else if (isHovered)
			drawList->AddCircleFilled(center, size * 0.5f, ImGui::GetColorU32(buttonHovered));

		drawList->AddImage((ImTextureID)(uint64_t)icon->GetRendererID(), pMin, pMax, ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));

		ImGui::End();
		ImGui::PopStyleVar(2);
	}
}