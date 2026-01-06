#include <Runic2D.h>
#include <Runic2D/Core/EntryPoint.h>

#include "../../Projects/Survivor/Assets/scripts/ScriptRegistry.h"

#include "Sandbox2D.h"

class SandboxApp : public Runic2D::Application
{
public:
	SandboxApp()
	{
		ScriptEngine::SetScriptBinder(ScriptRegistry::BindScript);
		ScriptEngine::SetScriptNamesGetter(ScriptRegistry::GetScriptNames);
		PushLayer(new Sandbox2D());
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