#include <Runic2D.h>
#include <Runic2D/Core/EntryPoint.h>

#include "Sandbox2D.h"
#include "UILayer.h"

class SandboxApp : public Runic2D::Application
{
public:
	SandboxApp()
	{
		//PushLayer(new Sandbox2D(m_GameContext));
		//PushOverlay(new UILayer(m_GameContext));
	}

	virtual ~SandboxApp()
	{
		// Destructor for SandboxApp, can be used for cleanup.
	}

private:
	std::shared_ptr<GameContext> m_GameContext;
};

Runic2D::Application* Runic2D::CreateApplication()
{
	// This function creates an instance of SandboxApp, which is the application to run.
	return new SandboxApp();
}