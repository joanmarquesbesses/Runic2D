#include "R2Dpch.h"
#include "ProjectSerializer.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Runic2D {

	ProjectSerializer::ProjectSerializer(Ref<Project> project)
		: m_Project(project)
	{
	}

	bool ProjectSerializer::Serialize(const std::filesystem::path& filepath)
	{
		const auto& config = m_Project->GetConfig();

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Project" << YAML::BeginMap;
		out << YAML::Key << "Name" << YAML::Value << config.Name;
		out << YAML::Key << "StartScene" << YAML::Value << config.StartScene;
		out << YAML::Key << "AssetDirectory" << YAML::Value << config.AssetDirectory;
		out << YAML::Key << "ScriptModulePath" << YAML::Value << config.ScriptModulePath;
		out << YAML::EndMap;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		if (!fout.is_open()) return false;
		fout << out.c_str();
		return true;
	}

	bool ProjectSerializer::Deserialize(const std::filesystem::path& filepath)
	{
		YAML::Node data;
		try { data = YAML::LoadFile(filepath.string()); }
		catch (const YAML::Exception& e)
		{
			R2D_CORE_ERROR("ProjectSerializer: Error YAML '{0}': {1}",
				filepath.string(), e.what());
			return false;
		}

		auto projectNode = data["Project"];
		if (!projectNode) return false;

		auto& config = m_Project->GetConfigMut();
		config.Name = projectNode["Name"].as<std::string>("Untitled");
		config.StartScene = projectNode["StartScene"].as<std::string>("");
		config.AssetDirectory = projectNode["AssetDirectory"].as<std::string>("Assets");
		config.ScriptModulePath = projectNode["ScriptModulePath"].as<std::string>("");
		return true;
	}
}
