#include "R2Dpch.h"
#include "AssetRegistry.h"

#include "Runic2D/Core/Threading/BackgroundTaskSystem.h"
#include "Runic2D/Project/Project.h"

#include <yaml-cpp/yaml.h> 
#include <fstream>

namespace Runic2D {

    std::unordered_map<UUID, std::filesystem::path> AssetRegistry::s_Registry;
    std::unordered_map<std::string, UUID> AssetRegistry::s_InverseRegistry;

    // Helper intern per normalitzar la ruta i treure si es o no de l'Engine
    static std::filesystem::path NormalizePath(const std::filesystem::path& filepath, std::filesystem::path* outBasePath = nullptr)
    {
        std::filesystem::path resolvedPath = filepath;
        bool isEngineResource = false;

        if (!filepath.is_absolute()) {
            if (Project::GetActive() && std::filesystem::exists(Project::GetAssetDirectory() / filepath)) {
                resolvedPath = Project::GetAssetDirectory() / filepath;
            } else if (std::filesystem::exists(Project::GetEngineResourcesDirectory().parent_path() / filepath)) {
                resolvedPath = Project::GetEngineResourcesDirectory().parent_path() / filepath;
                isEngineResource = true;
            } else {
                resolvedPath = std::filesystem::absolute(filepath);
            }
        }

        std::filesystem::path basePath;
        
        if (isEngineResource || (!Project::GetActive())) {
            basePath = Project::GetEngineResourcesDirectory().parent_path();
        } else {
            std::string resString = resolvedPath.string();
            std::string projString = Project::GetAssetDirectory().string();
            if (resString.find(projString) != std::string::npos) {
                basePath = Project::GetAssetDirectory();
            } else {
                basePath = Project::GetEngineResourcesDirectory().parent_path();
            }
        }

        if (outBasePath) *outBasePath = basePath;

        return std::filesystem::relative(resolvedPath, basePath);
    }

    void AssetRegistry::RegisterAsset(UUID uuid, const std::filesystem::path& filepath)
    {
        std::filesystem::path basePath;
        std::filesystem::path relativePath = NormalizePath(filepath, &basePath);
        std::string pathString = relativePath.string();
        std::replace(pathString.begin(), pathString.end(), '\\', '/');

        // Write .meta file
        std::filesystem::path metaPath = basePath / relativePath;
        metaPath = metaPath.string() + ".meta";

        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Asset" << YAML::Value << YAML::BeginMap;
        out << YAML::Key << "UUID" << YAML::Value << (uint64_t)uuid;
        out << YAML::EndMap;
        out << YAML::EndMap;

        std::ofstream fout(metaPath);
        fout << out.c_str();

        s_Registry[uuid] = pathString;
        s_InverseRegistry[pathString] = uuid;
    }

    std::filesystem::path AssetRegistry::GetFilepath(UUID uuid)
    {
        if (s_Registry.find(uuid) != s_Registry.end())
            return s_Registry[uuid];

        return "";
    }

    UUID AssetRegistry::GetUUID(const std::filesystem::path& filepath)
    {
        std::filesystem::path relativePath = NormalizePath(filepath);
        std::string pathString = relativePath.string();
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

    void AssetRegistry::Serialize(const std::filesystem::path& assetDirectory)
    {
        // Ja no utilitzem AssetRegistry.r2dregistry
        // Els arxius .meta s'escriuen al moment a RegisterAsset
    }

    void AssetRegistry::Deserialize(const std::filesystem::path& assetDirectory)
    {
        Clear();
        if (!std::filesystem::exists(assetDirectory)) return;

        // Pas 1: Carregar tots els .meta existents
        auto scanDirectory = [](const std::filesystem::path& dir, const std::filesystem::path& basePath) {
            if (!std::filesystem::exists(dir)) return;
            for (auto& entry : std::filesystem::recursive_directory_iterator(dir))
            {
                if (!entry.is_directory() && entry.path().extension() == ".meta")
                {
                    try {
                        YAML::Node data = YAML::LoadFile(entry.path().string());
                        if (auto assetNode = data["Asset"])
                        {
                            UUID uuid = assetNode["UUID"].as<uint64_t>();
                            
                            // Trobem l'arxiu original llevant ".meta"
                            std::string originalPathStr = entry.path().string();
                            originalPathStr = originalPathStr.substr(0, originalPathStr.length() - 5);
                            std::filesystem::path originalPath(originalPathStr);

                            std::filesystem::path relativePath = std::filesystem::relative(originalPath, basePath);
                            std::string pathString = relativePath.string();
                            std::replace(pathString.begin(), pathString.end(), '\\', '/');

                            s_Registry[uuid] = pathString;
                            s_InverseRegistry[pathString] = uuid;
                        }
                    }
                    catch (YAML::ParserException& e) {
                        R2D_CORE_ERROR("Failed to load .meta file '{0}'\n     {1}", entry.path().string(), e.what());
                    }
                }
            }
        };

        scanDirectory(assetDirectory, assetDirectory);
        scanDirectory(Project::GetEngineResourcesDirectory(), Project::GetEngineResourcesDirectory().parent_path());

        // Pas 2: Generar agressivament els .meta que faltin per qualsevol fitxer que no sigui .meta
        auto generateMissingMetas = [](const std::filesystem::path& dir) {
            if (!std::filesystem::exists(dir)) return;

            auto isAssetExtension = [](const std::string& ext) {
                return ext == ".png" || ext == ".jpg" || ext == ".jpeg" ||
                       ext == ".wav" || ext == ".mp3" || ext == ".ogg" ||
                       ext == ".ttf" || ext == ".r2dscene" || ext == ".r2dprefab" ||
                       ext == ".glsl";
            };

            for (auto& entry : std::filesystem::recursive_directory_iterator(dir))
            {
                if (!entry.is_directory() && entry.path().extension() != ".meta")
                {
                    if (isAssetExtension(entry.path().extension().string()))
                    {
                        std::string metaPathStr = entry.path().string() + ".meta";
                        if (!std::filesystem::exists(metaPathStr))
                        {
                            // Si l'arxiu no te .meta, l'hi creem al moment amb un UUID nou
                            UUID newUUID = UUID();
                            RegisterAsset(newUUID, entry.path());
                            R2D_CORE_INFO("Generat arxiu .meta automatic per a: {0}", entry.path().filename().string());
                        }
                    }
                }
            }
        };

        generateMissingMetas(assetDirectory);
        generateMissingMetas(Project::GetEngineResourcesDirectory());
    }
}
