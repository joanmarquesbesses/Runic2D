#pragma once

#include "Core.h"
#include "Events/Event.h"
#include "Runic2D/Events/ApplicationEvent.h"

#include "Window.h"

namespace Runic2D
{
	class RUNIC2D_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);
	private:
		bool OnWindowClose(WindowCloseEvent& e)
		{
			m_Running = false;
			return true;
		}

		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
	};

	//  To be defined in the client
	Application* CreateApplication();

}