#pragma once

#include "Core.h"

#include "Window.h"
#include "Runic2D/LayerStack.h"
#include "Runic2D/Events/ApplicationEvent.h"
#include "ImGui/ImGuiLayer.h"

namespace Runic2D
{
	class RUNIC2D_API Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }
	private:
		bool OnWindowClose(WindowCloseEvent& e)
		{
			m_Running = false;
			return true;
		}

		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		LayerStack m_LayerStack;
	private:
		static Application* s_Instance;
	};

	//  To be defined in the client
	Application* CreateApplication();

}