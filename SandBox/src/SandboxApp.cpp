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

        m_ActiveScene = Runic2D::CreateRef<Runic2D::Scene>();

        PushLayer(new GameplayLayer(m_ActiveScene));
    }

    virtual ~SandboxApp()
    {
        m_ActiveScene = nullptr;
        Runic2D::Project::UnloadRuntimeLibrary();
    }

private:
	Runic2D::Ref<Runic2D::Scene> m_ActiveScene;
};

Runic2D::Application* Runic2D::CreateApplication()
{
	// This function creates an instance of SandboxApp, which is the application to run.
	return new SandboxApp();
}