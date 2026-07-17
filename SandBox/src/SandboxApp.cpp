#include <Runic2D.h>
#include <Runic2D/Core/App/EntryPoint.h>

#include "GameplayLayer.h"

class SandboxApp : public Runic2D::Application
{
public:
    SandboxApp()
    {
        // Intentem carregar relatiu al directori de treball (Editor / VS)
        if (!Runic2D::Project::Load("Projects/Survivor/Survivor.r2dproj"))
        {
            // Fallback per si executem el .exe des de la carpeta bin/Dist
            R2D_CORE_WARN("No s'ha trobat el projecte a la ruta per defecte. Intentant ruta alternativa...");
            if (!Runic2D::Project::Load("../../../Projects/Survivor/Survivor.r2dproj"))
            {
                R2D_CORE_ERROR("SandboxApp: No s'ha pogut carregar el projecte Survivor!");
            }
        }
        
        if (Runic2D::Project::GetActive())
        {
            Runic2D::Project::LoadRuntimeLibrary();
        }

        std::string gameName = Runic2D::Project::GetConfig().Name;
        Runic2D::Application::Get().GetWindow().SetTitle(gameName);

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
