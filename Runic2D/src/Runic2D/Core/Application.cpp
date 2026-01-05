#include "R2Dpch.h"
#include "Application.h"

#include "Core.h"
#include "Runic2D/Renderer/Renderer.h"
#include "Runic2D/Assets/ResourceManager.h"
#include "Runic2D/Audio/AudioEngine.h"
#include "Runic2D/Utils/Random.h"

#include "Input.h"

#include <GLFW/glfw3.h>

namespace Runic2D {

	Application* Application::s_Instance = nullptr;

	Application::Application(const std::string& name)
	{
		R2D_PROFILE_FUNCTION();

		R2D_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
		R2D_CORE_INFO("Runic2D Engine Initialized");

		m_Window = std::unique_ptr<Window>(Window::Create(WindowProps(name)));
		m_Window->SetEventCallback(R2D_BIND_EVENT_FN(OnEvent));

		Random::Init();
		Renderer::Init();
		AudioEngine::Init();

#ifndef R2D_DIST
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
#endif
	}

	Application::~Application()
	{
		R2D_PROFILE_FUNCTION();
		for (Layer* layer : m_LayerStack) {
			layer->OnDetach();
		}
		Renderer::Shutdown();
		AudioEngine::Shutdown();
		ResourceManager::Clear();
	}

	void Application::PushLayer(Layer* layer)
	{
		R2D_PROFILE_FUNCTION();

		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		R2D_PROFILE_FUNCTION();

		m_LayerStack.PushOverlay(overlay);
		overlay->OnAttach();
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		R2D_PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0) {
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}

	void Application::Close()
	{
		m_Running = false;
	}

	void Application::OnEvent(Event& e)
	{
		R2D_PROFILE_FUNCTION();

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(R2D_BIND_EVENT_FN(OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(R2D_BIND_EVENT_FN(OnWindowResize));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();) {
			(*--it)->OnEvent(e);
			if (e.IsHandled())
				break;
		}
	}

	void Application::Run()
	{
		R2D_PROFILE_FUNCTION();

		while (m_Running) {

			R2D_PROFILE_SCOPE("RunLoop");

			float time = (float)glfwGetTime(); //TODO: Platform class
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			m_FrameTimeAccumulator += timestep.GetSeconds();
			m_FrameCount++;

			if (m_FrameTimeAccumulator >= 1.0f) { 
				m_AverageFPS = m_FrameCount / m_FrameTimeAccumulator;
				m_FrameTimeAccumulator = 0.0f; 
				m_FrameCount = 0;              
			}

			if (!m_Minimized) {
				{
					R2D_PROFILE_SCOPE("LayerStack OnUpdate");
					for (Layer* layer : m_LayerStack) {
						layer->OnUpdate(timestep);
					}
				}
#ifndef R2D_DIST
				m_ImGuiLayer->Begin();
				{
					R2D_PROFILE_SCOPE("LayerStack OnImGuiRender");
					for (Layer* layer : m_LayerStack) {
						layer->OnImGuiRender();
					}
				}
				m_ImGuiLayer->End();
#endif
			}

			m_Window->OnUpdate();
		}
	}

}
