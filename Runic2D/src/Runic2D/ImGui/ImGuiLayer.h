#pragma once

#include "Runic2D/Core/Layer.h"

#include "Runic2D/Events/ApplicationEvent.h"
#include "Runic2D/Events/KeyEvent.h"
#include "Runic2D/Events/MouseEvent.h"

namespace Runic2D
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;

		void Begin();
		void End();

		void SetBlockEvents(bool block) { m_BlockEvents = block; }
	private:
		void SetDarkThemeColors();
	private:
		bool m_BlockEvents = true;
		float m_Time = 0.0f;
	};
}