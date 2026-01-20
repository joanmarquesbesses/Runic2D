#include <Runic2D.h>
#include <Runic2D/Core/EntryPoint.h>

#include "../../Projects/Survivor/Assets/scripts/ScriptRegistry.h"

#include "EditorLayer.h"

namespace Runic2D
{
	class Runic2DEditor : public Application
	{
	public:
		Runic2DEditor()
			: Application("Runic2D Editor")
		{
			// Constructor for SandboxApp, can be used to initialize the application.
			//PushLayer(new ExampleLayer()); // Adding an instance of ExampleLayer to the application.
			ScriptEngine::SetScriptBinder(ScriptRegistry::BindScript);
			ScriptEngine::SetScriptNamesGetter(ScriptRegistry::GetScriptNames);
			PushLayer(new EditorLayer());
		}

		virtual ~Runic2DEditor()
		{
			// Destructor for SandboxApp, can be used for cleanup.
		}
	};

	Application* Runic2D::CreateApplication()
	{
		// This function creates an instance of SandboxApp, which is the application to run.
		return new Runic2DEditor();
	}
}