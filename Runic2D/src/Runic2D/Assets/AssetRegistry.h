#pragma once

#include "Runic2D/Core/Base/UUID.h"
#include <unordered_map>
#include <string>
#include <filesystem>

namespace Runic2D {

    class RUNIC_API AssetRegistry
    {
    public:
        static void RegisterAsset(UUID uuid, const std::filesystem::path& filepath);
        static std::filesystem::path GetFilepath(UUID uuid);
        static UUID GetUUID(const std::filesystem::path& filepath);

        static bool Contains(UUID uuid);
        static void Clear();

        static void Serialize(const std::filesystem::path& filepath);
        static void Deserialize(const std::filesystem::path& filepath);

    private:
        static std::unordered_map<UUID, std::filesystem::path> s_Registry;
        static std::unordered_map<std::string, UUID> s_InverseRegistry;
    };
}
