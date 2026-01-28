#include <Runic2D.h>
#include <Runic2D/Core/EntryPoint.h>

#include "../../Projects/Survivor/Assets/scripts/ScriptRegistry.h"
#include "../../Projects/Survivor/Assets/scripts/GameContext.h"

#include "Sandbox2D.h"
#include "UILayer.h"

class SandboxApp : public Runic2D::Application
{
public:
	SandboxApp()
	{
		m_GameContext = std::make_shared<GameContext>();

		ScriptEngine::SetScriptBinder(ScriptRegistry::BindScript);
		ScriptEngine::SetScriptNamesGetter(ScriptRegistry::GetScriptNames);

		PushLayer(new Sandbox2D(m_GameContext));
		PushOverlay(new UILayer(m_GameContext));
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