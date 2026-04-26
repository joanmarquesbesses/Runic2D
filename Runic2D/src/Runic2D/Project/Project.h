#pragma once

#include <string>
#include <filesystem>
#include "Runic2D/Core/Core.h"

namespace Runic2D {

	struct ProjectConfig
	{
		std::string Name = "Untitled";
		std::string StartScene = "";   // relatiu a AssetDirectory
		std::string AssetDirectory = "Assets";
		std::string ScriptModulePath = ""; // relatiu a l'arrel del projecte
	};

	class RUNIC_API Project
	{
    public:
        static Ref<Project>           GetActive() { return s_ActiveProject; }
        static const ProjectConfig&   GetConfig() { return s_ActiveProject->m_Config; }
        static const std::filesystem::path& GetProjectDirectory() { return s_ActiveProject->m_ProjectDirectory; }

        static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& relativePath)
        {
            R2D_CORE_ASSERT(s_ActiveProject, "No hi ha cap projecte actiu!");
            return s_ActiveProject->m_ProjectDirectory
                / s_ActiveProject->m_Config.AssetDirectory
                / relativePath;
        }

        static std::filesystem::path GetAssetDirectory()
        {
            R2D_CORE_ASSERT(s_ActiveProject, "No hi ha cap projecte actiu!");
            return s_ActiveProject->m_ProjectDirectory / s_ActiveProject->m_Config.AssetDirectory;
        }

        static bool Load(const std::filesystem::path& filepath);
        static bool Save(const std::filesystem::path& filepath);
        static Ref<Project> New();

        static void LoadRuntimeLibrary();
        static void UnloadRuntimeLibrary();

        ProjectConfig& GetConfigMut() { return m_Config; }

        static void Shutdown()
        {
            s_ActiveProject = nullptr;
        }

    private:
        ProjectConfig          m_Config;
        std::filesystem::path  m_ProjectDirectory; 
        static Ref<Project>    s_ActiveProject;
	};
}
