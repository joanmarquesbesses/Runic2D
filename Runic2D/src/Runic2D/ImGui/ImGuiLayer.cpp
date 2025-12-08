#include "R2Dpch.h"
#include "ImGuiLayer.h"

#include "backends/imgui_impl_opengl3.h"
#include "backends/imgui_impl_glfw.h"


#include "Runic2D/Core/Application.h"

//Temporary includes for ImGui
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "ImGuizmo.h"

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
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

		io.Fonts->Clear();
		io.FontDefault = io.Fonts->AddFontFromFileTTF("assets/fonts/Inter/Inter_18pt-Regular.ttf", 16.0f);
		io.Fonts->AddFontFromFileTTF("assets/fonts/Inter/Inter_18pt-Bold.ttf", 16.0f);

		//Style setup
		SetDarkThemeColors(); // Set ImGui style

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
		ImGuizmo::BeginFrame();
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
	void ImGuiLayer::SetDarkThemeColors()
	{
		auto& colors = ImGui::GetStyle().Colors;

		// --- ESTIL DE LA FINESTRA (Arondejar cantonades) ---
		auto& style = ImGui::GetStyle();
		style.WindowRounding = 4.0f; // Finestres amb puntes suaus
		style.FrameRounding = 4.0f;  // Botons i Inputs aroniejats
		style.PopupRounding = 4.0f;
		style.ScrollbarSize = 12.0f;
		style.ScrollbarRounding = 12.0f;
		style.GrabRounding = 4.0f;
		style.TabRounding = 4.0f;

		// --- COLORS ---

		// Fons general de les finestres (Gris fosc, gairebé negre)
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.105f, 0.11f, 1.0f };

		// Headers (La barra de títol que deies "blava")
		// La posem gris fosc perquè es fusioni amb la finestra o destaqui poc
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Botons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Frame BG (Fons dels Inputs de text, checkbox, sliders)
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs (Pestanyes)
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

		// Title Bar (La barra de dalt de tot de cada panell)
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
	}

	
} // namespace Runic2D