#include "R2Dpch.h"
#include "Application.h"

#include "Runic2D/Events/ApplicationEvent.h"

#include <GLFW/glfw3.h>

namespace Runic2D {
	Application::Application()
	{
		R2D_CORE_INFO("Runic2D Engine Initialized");

		m_Window = std::unique_ptr<Window>(Window::Create());
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
}
