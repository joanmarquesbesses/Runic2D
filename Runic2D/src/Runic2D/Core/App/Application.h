#pragma once

#include "Runic2D/Core/Base/Core.h"

#include "Window.h"
#include "Runic2D/Core/App/LayerStack.h"
#include "Runic2D/Events/ApplicationEvent.h"

#include "Runic2D/Core/Base/Timestep.h"

#include "Runic2D/ImGui/ImGuiLayer.h"

#include <vector>
#include <mutex>
#include <functional>

namespace Runic2D
{
	class RUNIC_API Application
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
		float GetAverageFrameTimeMs() const { return m_AverageFrameTimeMs; }
		float GetFixedUpdateAlpha() const { return m_FixedUpdateAccumulator / (1.0f / 60.0f); }

		void SubmitToMainThread(const std::function<void()>& function);
	private:
		void ExecuteMainThreadQueue();

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
		float m_FixedUpdateAccumulator = 0.0f;
		int m_FrameCount = 0;               
		float m_AverageFPS = 0.0f;
		float m_AverageFrameTimeMs = 0.0f;

		std::vector<std::function<void()>> m_MainThreadQueue;
		std::mutex m_MainThreadQueueMutex;
	private:
		static Application* s_Instance;
	};

	//  To be defined in the client
	Application* CreateApplication();

}
