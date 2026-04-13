#include "R2Dpch.h"
#include "Project.h"
#include "ProjectSerializer.h"

#include "Runic2D/Scripting/ScriptEngine.h"

#include <Windows.h>

namespace Runic2D {

	Ref<Project> Project::New()
	{
		s_ActiveProject = CreateRef<Project>();
		return s_ActiveProject;
	}

	Ref<Project> Project::Load(const std::filesystem::path& path)
	{
		Ref<Project> project = CreateRef<Project>();

		ProjectSerializer serializer(project);
		if (serializer.Deserialize(path))
		{
			project->m_ProjectDirectory = path.parent_path();
			s_ActiveProject = project;
			return s_ActiveProject;
		}

		return nullptr;
	}

	bool Project::SaveActive(const std::filesystem::path& path)
	{
		ProjectSerializer serializer(s_ActiveProject);
		if (serializer.Serialize(path))
		{
			s_ActiveProject->m_ProjectDirectory = path.parent_path();
			return true;
		}
		return false;
	}

	bool Project::LoadRuntimeLibrary()
	{
		if (s_RuntimeLibraryHandle) {
			FreeLibrary((HMODULE)s_RuntimeLibraryHandle);
			s_RuntimeLibraryHandle = nullptr;
		}

#ifdef R2D_DEBUG
		std::string config = "Debug";
#elif R2D_RELEASE
		std::string config = "Release";
#else
		std::string config = "Dist";
#endif

		std::string dllPath = "bin/" + config + "-windows-x86_64/Survivor/Survivor.dll";

		HMODULE hLib = LoadLibraryA(dllPath.c_str());
		if (!hLib) {
			R2D_CORE_ERROR("Project: Game DLL couldn't be loaded {0}", dllPath);
			return false;
		}

		s_RuntimeLibraryHandle = hLib;

		typedef void (*InitFn)();
		InitFn initModule = (InitFn)GetProcAddress(hLib, "InitRuntimeModule");

		if (initModule) {
			initModule(&GameContext::Get()); // Això crida al teu ScriptRegistry::BindScript
			return true;
		}

		R2D_CORE_ERROR("Project: The DLL doesn't have InitRuntimeModule function!");
		return false;
	}
}