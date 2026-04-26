#include <Runic2D.h>
#include <Runic2D/Core/EntryPoint.h>

#include "GameplayLayer.h"

class SandboxApp : public Runic2D::Application
{
public:
    SandboxApp()
    {
        if (Runic2D::Project::Load("Projects/Survivor/Survivor.r2dproj")) {
            Runic2D::Project::LoadRuntimeLibrary();
        }

        PushLayer(new GameplayLayer());
    }

    virtual ~SandboxApp()
    {
        Runic2D::SceneManager::Shutdown();
        Runic2D::Project::Shutdown();
    }

private:
	Runic2D::Ref<Runic2D::Scene> m_ActiveScene;
};

Runic2D::Application* Runic2D::CreateApplication()
{
	// This function creates an instance of SandboxApp, which is the application to run.
	return new SandboxApp();
}