#pragma once
#include "Runic2D/Renderer/Texture.h"
#include <functional>

namespace Runic2D {

	enum class SceneState { Edit = 0, Play = 1 };

	class ToolbarPanel
	{
	public:
		ToolbarPanel();

		void OnImGuiRender(SceneState state);

		// Callbacks per comunicar-se amb l'EditorLayer
		void SetOnPlayCallback(const std::function<void()>& callback) { m_OnPlayCallback = callback; }
		void SetOnStopCallback(const std::function<void()>& callback) { m_OnStopCallback = callback; }

	private:
		Ref<Texture2D> m_IconPlay;
		Ref<Texture2D> m_IconStop;

		std::function<void()> m_OnPlayCallback;
		std::function<void()> m_OnStopCallback;
	};
}
