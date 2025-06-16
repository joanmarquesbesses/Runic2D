#pragma once

#include "Runic2D/Layer.h"

#include "Runic2D/Events/ApplicationEvent.h"
#include "Runic2D/Events/KeyEvent.h"
#include "Runic2D/Events/MouseEvent.h"

namespace Runic2D
{
	class RUNIC2D_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach();
		void OnDetach();
		void OnUpdate();
		void OnEvent(Event& event);

	private:
		// Add any private members if needed
		float m_Time = 0.0f;
	};
}