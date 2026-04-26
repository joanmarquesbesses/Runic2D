#include "R2Dpch.h"
#include "Project.h"
#include "ProjectSerializer.h"

#ifdef R2D_PLATFORM_WINDOWS
	#include <Windows.h>
	static HMODULE s_GameDLL = nullptr;
#endif

namespace Runic2D {

	Ref<Project> Project::s_ActiveProject = nullptr;

	Ref<Project> Project::New()
	{
		s_ActiveProject = CreateRef<Project>();
		return s_ActiveProject;
	}

	bool Project::Load(const std::filesystem::path& filepath)
	{
		auto project = CreateRef<Project>();
		ProjectSerializer serializer(project);

		if (!serializer.Deserialize(filepath))
		{
			R2D_CORE_ERROR("Project: No s'ha pogut carregar '{0}'", filepath.string());
			return false;
		}

		project->m_ProjectDirectory = filepath.parent_path();
		s_ActiveProject = project;

		R2D_CORE_INFO("Project carregat: '{0}' des de '{1}'",
			project->m_Config.Name, filepath.string());
		return true;
	}

	bool Project::Save(const std::filesystem::path& filepath)
	{
		R2D_CORE_ASSERT(s_ActiveProject, "No hi ha projecte actiu per guardar!");
		ProjectSerializer serializer(s_ActiveProject);
		if (!serializer.Serialize(filepath)) return false;
		s_ActiveProject->m_ProjectDirectory = filepath.parent_path();
		R2D_CORE_INFO("Project guardat: '{0}'", filepath.string());
		return true;
	}

	void Project::LoadRuntimeLibrary()
	{
#ifdef R2D_PLATFORM_WINDOWS
		R2D_CORE_ASSERT(s_ActiveProject, "No hi ha projecte actiu!");
		if (s_GameDLL) UnloadRuntimeLibrary();

		std::string configName;
#if defined(R2D_DEBUG)
		configName = "Debug-windows-x86_64";
#elif defined(R2D_RELEASE)
		configName = "Release-windows-x86_64";
#elif defined(R2D_DIST)
		configName = "Dist-windows-x86_64";
#endif

		std::filesystem::path dllPath = std::filesystem::current_path()
			/ "bin" / configName
			/ s_ActiveProject->m_Config.Name
			/ s_ActiveProject->m_Config.ScriptModulePath;

		// Normalitzar per evitar //..// etc.
		dllPath = std::filesystem::weakly_canonical(dllPath);

		s_GameDLL = LoadLibraryA(dllPath.string().c_str());
		if (!s_GameDLL)
		{
			R2D_CORE_ERROR("Project: No s'ha pogut carregar la DLL '{0}' (error: {1})",
				dllPath.string(), GetLastError());
			return;
		}

		using InitFn = void(*)();
		auto initFn = (InitFn)GetProcAddress(s_GameDLL, "InitRuntimeModule");
		if (initFn) initFn();

		R2D_CORE_INFO("Project: DLL carregada correctament: '{0}'", dllPath.string());
#endif
	}

	void Project::UnloadRuntimeLibrary()
	{
#ifdef R2D_PLATFORM_WINDOWS
		if (!s_GameDLL) return;

		using ShutdownFn = void(*)();
		auto shutdownFn = (ShutdownFn)GetProcAddress(s_GameDLL, "ShutdownRuntimeModule");
		if (shutdownFn) shutdownFn();

		FreeLibrary(s_GameDLL);
		s_GameDLL = nullptr;
		R2D_CORE_INFO("Project: DLL descarregada.");
#endif
	}
}