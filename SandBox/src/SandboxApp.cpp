#include <Runic2D.h>
#include <Runic2D/Core/App/EntryPoint.h>

#include "GameplayLayer.h"

class SandboxApp : public Runic2D::Application
{
public:
    SandboxApp()
    {
        bool projectLoaded = false;
        
        // Cerca qualsevol arxiu .r2dproj a l'arrel
        for (const auto& entry : std::filesystem::directory_iterator("."))
        {
            if (entry.path().extension() == ".r2dproj")
            {
                if (Runic2D::Project::Load(entry.path().string()))
                {
                    projectLoaded = true;
                    break;
                }
            }
        }

        if (!projectLoaded)
        {
            // Fallback: cerca a un nivell superior per si l'estem executant des d'una carpeta niada
            for (const auto& entry : std::filesystem::directory_iterator(".."))
            {
                if (entry.path().extension() == ".r2dproj")
                {
                    if (Runic2D::Project::Load(entry.path().string()))
                    {
                        projectLoaded = true;
                        break;
                    }
                }
            }
        }

        if (!projectLoaded)
        {
            // Fallback: per si estem compilant el motor original aïllat
            if (Runic2D::Project::Load("Projects/Survivor/Survivor.r2dproj"))
            {
                projectLoaded = true;
            }
        }

        if (!projectLoaded)
        {
            R2D_CORE_ERROR("SandboxApp: No s'ha trobat cap arxiu .r2dproj a la carpeta actual o pare!");
            exit(1);
        }
        
        if (Runic2D::Project::GetActive())
        {
            Runic2D::Project::LoadRuntimeLibrary();
        }

        std::string gameName = "Runic2D Engine";
        if (Runic2D::Project::GetActive())
            gameName = Runic2D::Project::GetConfig().Name;
            
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
