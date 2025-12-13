#pragma once

#include "Runic2D/Renderer/EditorCamera.h"
#include "ContentBrowserPanel.h"

namespace Runic2D {

	class SettingsPanel
	{
	public:
		SettingsPanel() = default;

		// Passem punters als objectes que volem configurar
		void OnImGuiRender(EditorCamera& camera, ContentBrowserPanel& contentBrowser, int& gizmoType, int& gizmoMode);
	};
}