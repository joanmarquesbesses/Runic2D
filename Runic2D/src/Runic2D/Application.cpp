#include "R2Dpch.h"
#include "Application.h"

#include <GLFW/glfw3.h>

namespace Runic2D {

#define R2D_BIND_EVENT_FN(fn) std::bind(&Application::fn, this, std::placeholders::_1)

	Application::Application()
	{
		R2D_CORE_INFO("Runic2D Engine Initialized");

		m_Window = std::unique_ptr<Window>(Window::Create());
		m_Window->SetEventCallback(R2D_BIND_EVENT_FN(OnEvent));
	}

	Application::~Application()
	{
	}

	void Application::Run()
	{
		while (m_Running) {
			glClearColor(1.0f, 1.0f, 0.5f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			m_Window->OnUpdate();
		}
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(R2D_BIND_EVENT_FN(OnWindowClose));

		R2D_CORE_TRACE("Event: {0}", e);
	}
}
