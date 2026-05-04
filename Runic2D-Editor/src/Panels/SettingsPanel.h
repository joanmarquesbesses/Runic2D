#pragma once

#include "Runic2D/Renderer/EditorCamera.h"
#include "ContentBrowserPanel.h"

namespace Runic2D {

	class Scene;

	class SettingsPanel
	{
	public:
		SettingsPanel() = default;

		// Passem punters als objectes que volem configurar
		void OnImGuiRender(Ref<Scene> activeScene, EditorCamera& camera, ContentBrowserPanel& contentBrowser, int& gizmoType, int& gizmoMode, bool& showColliders);
	};
}