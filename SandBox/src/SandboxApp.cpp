#include <Runic2D.h>

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
		R2D_INFO("ExampleLayer::OnUpdate - This method is called every frame to update the layer.");
	}

	void OnEvent(Runic2D::Event& event) override
	{
		R2D_TRACE("ExampleLayer::OnEvent - Event received: {0}", event);
	}
};

class SandboxApp : public Runic2D::Application
{
public:
	SandboxApp()
	{
		// Constructor for SandboxApp, can be used to initialize the application.
		PushLayer(new ExampleLayer()); // Adding an instance of ExampleLayer to the application.
		PushOverlay(new Runic2D::ImGuiLayer()); // Adding ImGuiLayer for GUI rendering.
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