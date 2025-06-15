#include "R2Dpch.h"
#include "Application.h"

#include <glad/glad.h>

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

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushOverlay(overlay);
	}

	void Application::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(R2D_BIND_EVENT_FN(OnWindowClose));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
			(*--it)->OnEvent(e);
			if (e.Handled())
				break;
		}
	}

	void Application::Run()
	{
		while (m_Running) {
			glClearColor(1.0f, 1.0f, 0.5f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			for (Layer* layer : m_LayerStack) {
				layer->OnUpdate();
			}

			m_Window->OnUpdate();
		}
	}

}
