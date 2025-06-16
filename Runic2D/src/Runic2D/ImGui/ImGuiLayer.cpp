#include "R2Dpch.h"
#include "ImGuiLayer.h"

#include "Platform/OpenGL/ImGuiOpenGLRenderer.h"

#include "Runic2D/Application.h"

//Temporary includes for ImGui
#include <GLFW/glfw3.h>

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
		// Code to attach the ImGui layer, such as initializing ImGui context
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();	 // Initialize ImGui IO
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors; // Enable mouse cursors
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos; // Enable setting mouse position

		// Set up key mappings
		io.AddKeyEvent(ImGuiKey_Tab, true);      // cuando se presiona
		io.AddKeyEvent(ImGuiKey_Tab, false);     // cuando se suelta

		ImGui_ImplOpenGL3_Init("#version 460"); // Initialize OpenGL renderer for ImGui
	}

	void ImGuiLayer::OnDetach()
	{
		// Code to detach the ImGui layer, such as shutting down ImGui context
	}

	void ImGuiLayer::OnUpdate()
	{
		ImGuiIO& io = ImGui::GetIO(); // Get ImGui IO instance
		Application& app = Application::Get(); // Get the application instance
		io.DisplaySize = ImVec2((float)Application::Get().GetWindow().GetWidth(), (float)Application::Get().GetWindow().GetHeight()); // Set display size for ImGui

		float time = (float)glfwGetTime(); // Get current time for ImGui
		io.DeltaTime = m_Time > 0.0f ? (time - m_Time) : (1.0f / 60.0f); // Calculate delta time
		m_Time = time; // Update the time for the next frame

		ImGui_ImplOpenGL3_NewFrame(); // Start a new ImGui frame
		ImGui::NewFrame(); // Begin a new ImGui frame

		static bool showDemoWindow = true;
		if (showDemoWindow)
			ImGui::ShowDemoWindow(&showDemoWindow); // Show ImGui demo window

		ImGui::Render(); // Render ImGui frame
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Render ImGui draw data using OpenGL
	}

	void ImGuiLayer::OnEvent(Event& event)
	{
		// Code to handle events in the ImGui layer
	}
} // namespace Runic2D