#include <Runic2D.h>

#include "imgui/imgui.h"

class ExampleLayer : public Runic2D::Layer
{
public:
	ExampleLayer()
		: Layer("Example")
	{
		// Constructor for ExampleLayer, can be used to initialize the layer.
	}

	void OnUpdate() override
	{
		//R2D_INFO("ExampleLayer::OnUpdate - This method is called every frame to update the layer.");

		if (Runic2D::Input::IsKeyPressed(R2D_KEY_P))
		{
			R2D_TRACE("ExampleLayer::OnUpdate - Key 'P' is pressed.");
		}
	}

	void OnEvent(Runic2D::Event& event) override
	{
		if (event.GetEventType() == Runic2D::EventType::KeyPressed)
		{
			auto& keyEvent = static_cast<Runic2D::KeyPressedEvent&>(event);
			R2D_TRACE("ExampleLayer::OnEvent - Key pressed: {0}", (char)keyEvent.GetKeyCode());
		}
	}

	void OnImGuiRender() override
	{
		// This method can be used to render ImGui elements for this layer.
		ImGui::Begin("Example Layer");
		ImGui::Text("Hello from ExampleLayer!");
		ImGui::End();
	}
};

class SandboxApp : public Runic2D::Application
{
public:
	SandboxApp()
	{
		// Constructor for SandboxApp, can be used to initialize the application.
		PushLayer(new ExampleLayer()); // Adding an instance of ExampleLayer to the application.
	}

	virtual ~SandboxApp()
	{
		// Destructor for SandboxApp, can be used for cleanup.
	}
};

Runic2D::Application* Runic2D::CreateApplication()
{
	// This function creates an instance of SandboxApp, which is the application to run.
	return new SandboxApp();
}