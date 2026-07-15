#include "R2Dpch.h"
#include "AssetRegistry.h"

#include "Runic2D/Core/BackgroundTaskSystem.h"
#include "Runic2D/Project/Project.h"

#include <yaml-cpp/yaml.h> 
#include <fstream>

namespace Runic2D {

    std::unordered_map<UUID, std::filesystem::path> AssetRegistry::s_Registry;
    std::unordered_map<std::string, UUID> AssetRegistry::s_InverseRegistry;

    void AssetRegistry::RegisterAsset(UUID uuid, const std::filesystem::path& filepath)
    {
        std::string pathString = filepath.string();
        std::replace(pathString.begin(), pathString.end(), '\\', '/');

        s_Registry[uuid] = pathString;
        s_InverseRegistry[pathString] = uuid;

        BackgroundTaskSystem::Execute(
            []() {
                if (Project::GetActive()) {
                    Serialize(Project::GetAssetDirectory() / "AssetRegistry.r2dregistry");
                }
            }
        );
    }

    std::filesystem::path AssetRegistry::GetFilepath(UUID uuid)
    {
        if (s_Registry.find(uuid) != s_Registry.end())
            return s_Registry[uuid];

        return "";
    }

    UUID AssetRegistry::GetUUID(const std::filesystem::path& filepath)
    {
        std::string pathString = filepath.string();
        std::replace(pathString.begin(), pathString.end(), '\\', '/');

        if (s_InverseRegistry.find(pathString) != s_InverseRegistry.end())
            return s_InverseRegistry[pathString];

        return 0;
    }

    bool AssetRegistry::Contains(UUID uuid)
    {
        return s_Registry.find(uuid) != s_Registry.end();
    }

    void AssetRegistry::Clear()
    {
        s_Registry.clear();
        s_InverseRegistry.clear();
    }

    void AssetRegistry::Serialize(const std::filesystem::path& filepath)
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "AssetRegistry" << YAML::Value << YAML::BeginMap;
        for (const auto& [uuid, path] : s_Registry) {
            out << YAML::Key << (uint64_t)uuid << YAML::Value << path.string();
        }
        out << YAML::EndMap;
        out << YAML::EndMap;
        std::ofstream fout(filepath);
        fout << out.c_str();
    }

    void AssetRegistry::Deserialize(const std::filesystem::path& filepath)
    {
        if (!std::filesystem::exists(filepath)) return;
        YAML::Node data = YAML::LoadFile(filepath.string());
        if (auto registryNode = data["AssetRegistry"]) {
            for (auto node : registryNode) {
                UUID uuid = node.first.as<uint64_t>();
                std::filesystem::path path = node.second.as<std::string>();

                s_Registry[uuid] = path;
                s_InverseRegistry[path.string()] = uuid;
            }
        }
    }
}