#pragma once

#include "Core.h"

#include "Window.h"
#include "Runic2D/Core/LayerStack.h"
#include "Runic2D/Events/ApplicationEvent.h"

#include "Runic2D/Core/Timestep.h"

#include "Runic2D/ImGui/ImGuiLayer.h"

namespace Runic2D
{
	class RUNIC2D_API Application
	{
	public:
		Application(const std::string& name = "Runic2D App");
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		inline static Application& Get() { return *s_Instance; }
		inline Window& GetWindow() { return *m_Window; }

		void Close();

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		float GetAverageFPS() const { return m_AverageFPS; }
	private:
		bool OnWindowClose(WindowCloseEvent& e)
		{
			m_Running = false;
			return true;
		}

		bool OnWindowResize(WindowResizeEvent& e);

		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;
		float m_LastFrameTime = 0.0f;
		float m_FrameTimeAccumulator = 0.0f;
		int m_FrameCount = 0;               
		float m_AverageFPS = 0.0f;
	private:
		static Application* s_Instance;
	};

	//  To be defined in the client
	Application* CreateApplication();

}