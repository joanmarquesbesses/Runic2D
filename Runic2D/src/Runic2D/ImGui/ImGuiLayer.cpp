#include "R2Dpch.h"
#include "ImGuiLayer.h"

#include "Platform/OpenGL/ImGuiOpenGLRenderer.h"


#include "Runic2D/Application.h"

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
		// Code to attach the ImGui layer, such as initializing ImGui context
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();	 // Initialize ImGui IO
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors; // Enable mouse cursors
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos; // Enable setting mouse position

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
		EventDispatcher dispatcher(event); // Create an event dispatcher for the current event
		dispatcher.Dispatch<MouseButtonPressedEvent>(RUNIC2D_BIND_EVENT_FN(ImGuiLayer::OnMouseButtonPressedEvent));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(RUNIC2D_BIND_EVENT_FN(ImGuiLayer::OnMouseButtonReleasedEvent));
		dispatcher.Dispatch<MouseMovedEvent>(RUNIC2D_BIND_EVENT_FN(ImGuiLayer::OnMouseMovedEvent));
		dispatcher.Dispatch<MouseScrolledEvent>(RUNIC2D_BIND_EVENT_FN(ImGuiLayer::OnMouseScrolledEvent));
		dispatcher.Dispatch<KeyPressedEvent>(RUNIC2D_BIND_EVENT_FN(ImGuiLayer::OnKeyPressedEvent));
		dispatcher.Dispatch<KeyReleasedEvent>(RUNIC2D_BIND_EVENT_FN(ImGuiLayer::OnKeyReleasedEvent));
		dispatcher.Dispatch<KeyTypedEvent>(RUNIC2D_BIND_EVENT_FN(ImGuiLayer::OnKeyTypedEvent));
		dispatcher.Dispatch<WindowResizeEvent>(RUNIC2D_BIND_EVENT_FN(ImGuiLayer::OnWindowResizeEvent));
	}

	bool ImGuiLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		if (e.GetMouseButton() >= 0 && e.GetMouseButton() < IM_ARRAYSIZE(io.MouseDown))
		{
			io.MouseDown[e.GetMouseButton()] = true; // Set mouse button state to pressed
		}

		return false;
	}

	bool ImGuiLayer::OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		if (e.GetMouseButton() >= 0 && e.GetMouseButton() < IM_ARRAYSIZE(io.MouseDown))
		{
			io.MouseDown[e.GetMouseButton()] = false; // Set mouse button state to released
		}
		return false;
	}

	bool ImGuiLayer::OnMouseMovedEvent(MouseMovedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(e.GetX(), e.GetY()); // Update mouse position in ImGui IO

		return false;
	}

	bool ImGuiLayer::OnMouseScrolledEvent(MouseScrolledEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.AddMouseWheelEvent(e.GetXOffset(), e.GetYOffset()); // Add mouse wheel scroll event to ImGui IO
		return false;
	}

	// Helper function to map GLFW keycodes to ImGui keycodes  
	int GLFWToImGuiKey(int glfwKey)
	{
		switch (glfwKey)
		{
		case GLFW_KEY_TAB: return ImGuiKey_Tab;
		case GLFW_KEY_LEFT: return ImGuiKey_LeftArrow;
		case GLFW_KEY_RIGHT: return ImGuiKey_RightArrow;
		case GLFW_KEY_UP: return ImGuiKey_UpArrow;
		case GLFW_KEY_DOWN: return ImGuiKey_DownArrow;
		case GLFW_KEY_PAGE_UP: return ImGuiKey_PageUp;
		case GLFW_KEY_PAGE_DOWN: return ImGuiKey_PageDown;
		case GLFW_KEY_HOME: return ImGuiKey_Home;
		case GLFW_KEY_END: return ImGuiKey_End;
		case GLFW_KEY_INSERT: return ImGuiKey_Insert;
		case GLFW_KEY_DELETE: return ImGuiKey_Delete;
		case GLFW_KEY_BACKSPACE: return ImGuiKey_Backspace;
		case GLFW_KEY_SPACE: return ImGuiKey_Space;
		case GLFW_KEY_ENTER: return ImGuiKey_Enter;
		case GLFW_KEY_ESCAPE: return ImGuiKey_Escape;
		case GLFW_KEY_A: return ImGuiKey_A;
		case GLFW_KEY_C: return ImGuiKey_C;
		case GLFW_KEY_V: return ImGuiKey_V;
		case GLFW_KEY_X: return ImGuiKey_X;
		case GLFW_KEY_Y: return ImGuiKey_Y;
		case GLFW_KEY_Z: return ImGuiKey_Z;
		default: return ImGuiKey_None;
		}
	}

	bool ImGuiLayer::OnKeyPressedEvent(KeyPressedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		int imguiKey = GLFWToImGuiKey(e.GetKeyCode()); // Convert GLFW keycode to ImGui keycode
		io.AddKeyEvent((ImGuiKey)imguiKey, true); // Add key event to ImGui

		io.AddKeyEvent(ImGuiKey_ModCtrl,
			glfwGetKey((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow(), GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
			glfwGetKey((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow(), GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);

		io.AddKeyEvent(ImGuiKey_ModShift,
			glfwGetKey((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
			glfwGetKey((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow(), GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

		io.AddKeyEvent(ImGuiKey_ModAlt,
			glfwGetKey((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow(), GLFW_KEY_LEFT_ALT) == GLFW_PRESS ||
			glfwGetKey((GLFWwindow*)Application::Get().GetWindow().GetNativeWindow(), GLFW_KEY_RIGHT_ALT) == GLFW_PRESS);

		return false;
	}

	bool ImGuiLayer::OnKeyReleasedEvent(KeyReleasedEvent& e)
	{
        ImGuiIO& io = ImGui::GetIO();  
        int imguiKey = GLFWToImGuiKey(e.GetKeyCode()); // Convert GLFW keycode to ImGui keycode  
        io.AddKeyEvent((ImGuiKey)imguiKey, false); // Add key release event to ImGui  

		GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
		io.AddKeyEvent(ImGuiKey_LeftCtrl, glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS);
		io.AddKeyEvent(ImGuiKey_RightCtrl, glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);

		return false;
	}

	bool ImGuiLayer::OnKeyTypedEvent(KeyTypedEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		int keyCode = e.GetKeyCode();
		if (keyCode > 0 && keyCode < 0x10000) // Check if key code is valid
		{
			io.AddInputCharacter((unsigned short)keyCode); // Add character input to ImGui IO
		}
		return false;
	}

	bool ImGuiLayer::OnWindowResizeEvent(WindowResizeEvent& e)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)e.GetWidth(), (float)e.GetHeight()); // Update display size in ImGui IO
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f); // Set framebuffer scale for high DPI displays
		glViewport(0, 0, e.GetWidth(), e.GetHeight()); // Update OpenGL viewport size

		return false;
	}
} // namespace Runic2D