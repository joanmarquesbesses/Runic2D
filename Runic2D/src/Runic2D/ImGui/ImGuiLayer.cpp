#include "R2Dpch.h"
#include "ImGuiLayer.h"

#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"


#include "Runic2D/Core/Application.h"

//Temporary includes for ImGui
#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Runic2D
{

	ImGuiLayer::ImGuiLayer()
		: Layer("ImGuiLayer")
	{
		// Initialization code for ImGui layer
	}

	ImGuiLayer::~ImGuiLayer()
	{
		// Cleanup code for ImGui layer
	}

	void ImGuiLayer::OnAttach()
	{
		R2D_PROFILE_FUNCTION();

		// Code to attach the ImGui layer, such as initializing ImGui context and setting up styles
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		//Style setup
		ImGui::StyleColorsDark(); // Set ImGui style to dark

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());

		//Set up ImGui for GLFW and OpenGL
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 330 core");
	}

	void ImGuiLayer::OnDetach()
	{
		R2D_PROFILE_FUNCTION();

		// Code to detach the ImGui layer, such as shutting down ImGui context
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::OnEvent(Event& e)
	{
		if (m_BlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (e.IsInCategory(EventCategoryMouse) && io.WantCaptureMouse)
				e.SetHandled(true);
			if (e.IsInCategory(EventCategoryKeyboard) && io.WantCaptureKeyboard)
				e.SetHandled(true);
		}
	}

	void ImGuiLayer::Begin()
	{
		R2D_PROFILE_FUNCTION();

		// Start a new ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		m_Time = (float)glfwGetTime();
	}

	void ImGuiLayer::End()
	{
		R2D_PROFILE_FUNCTION();

		// End the ImGui frame and render it
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());

		ImGui::Render(); // Render ImGui frame
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update platform windows if viewports are enabled
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backupCurrentContext = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backupCurrentContext);
		}
	}
} // namespace Runic2D