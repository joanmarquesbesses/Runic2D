#pragma once

#include <string>
#include <filesystem>
#include "Runic2D/Core/Core.h"

namespace Runic2D {

	struct ProjectConfig
	{
		std::string Name = "Untitled";

		std::filesystem::path StartScene;

		std::filesystem::path AssetDirectory = "Assets";
		std::filesystem::path ScriptModulePath; // Ignorar de moment
	};

	class Project
	{
	public:
		static const std::filesystem::path& GetAssetDirectory()
		{
			R2D_CORE_ASSERT(s_ActiveProject, "No active project!");
			return s_ActiveProject->m_Config.AssetDirectory;
		}

		static std::filesystem::path GetAssetFileSystemPath(const std::filesystem::path& path)
		{
			R2D_CORE_ASSERT(s_ActiveProject, "No active project!");
			auto assetDir = s_ActiveProject->m_Config.AssetDirectory;
			if (path.string().find(assetDir.string()) == 0)
			{
				return s_ActiveProject->m_ProjectDirectory / path;
			}

			return s_ActiveProject->m_ProjectDirectory / assetDir / path;
		}

		ProjectConfig& GetConfig() { return m_Config; }

		static Ref<Project> GetActive() { return s_ActiveProject; }

		static Ref<Project> New();
		static Ref<Project> Load(const std::filesystem::path& path);
		static bool SaveActive(const std::filesystem::path& path);

	private:
		ProjectConfig m_Config;
		std::filesystem::path m_ProjectDirectory;

		inline static Ref<Project> s_ActiveProject;
	};
}
