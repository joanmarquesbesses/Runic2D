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

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();

	private:
		float m_Time = 0.0f;
	};
}