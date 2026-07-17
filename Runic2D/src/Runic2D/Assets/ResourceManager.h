#pragma once

#include "Runic2D/Core/Base/Core.h"
#include "Runic2D/Core/Base/UUID.h"
#include "Runic2D/Project/Project.h" 
#include "Runic2D/Assets/AssetRegistry.h"
#include <unordered_map>
#include <string>
#include <filesystem>
#include <memory>
#include <functional>
#include <vector>
#include <mutex>

namespace Runic2D {

	class RUNIC_API ResourceManager
	{
	public:

		template<typename T>
		static Ref<T> Get(UUID uuid)
		{
			std::scoped_lock<std::mutex> lock(s_CacheMutex);
			auto& cache = GetCache<T>();

			if (cache.find(uuid) != cache.end())
			{
				return cache[uuid];
			}

			// Si no està en memòria, el busquem al Registre
			std::filesystem::path filepath = AssetRegistry::GetFilepath(uuid);
			std::filesystem::path resolvedPath = filepath; if (Project::GetActive() && !filepath.is_absolute()) { resolvedPath = Project::GetAssetFileSystemPath(filepath); } if (filepath.empty() || (!std::filesystem::exists(resolvedPath) && !std::filesystem::exists(filepath) && !std::filesystem::exists(Project::GetEngineResourcesDirectory().parent_path() / filepath)))
			{
				R2D_CORE_WARN("ResourceManager: Asset UUID {0} no trobat o ruta no vàlida", (uint64_t)uuid);
				return nullptr;
			}

			// Creem l'Asset i el guardem a la Cache mapejat per UUID
			std::string finalPath = filepath.string(); if (std::filesystem::exists(resolvedPath)) finalPath = resolvedPath.string(); else if (std::filesystem::exists(Project::GetEngineResourcesDirectory().parent_path() / filepath)) finalPath = (Project::GetEngineResourcesDirectory().parent_path() / filepath).string(); Ref<T> newAsset = T::Create(finalPath);
			newAsset->Handle = uuid;
			cache[uuid] = newAsset;

			return newAsset;
		}

		template<typename T>
		static Ref<T> Get(const std::filesystem::path& path)
		{
			UUID uuid = AssetRegistry::GetUUID(path);

			// Si la ruta no estava al registre, generem un UUID nou al vol i el registrem
			if (uuid == 0)
			{
				uuid = UUID();
				AssetRegistry::RegisterAsset(uuid, path);
			}

			// Ara que ja tenim l'UUID, utilitzem la via ràpida de dalt
			return Get<T>(uuid);
		}

		static void Clear();
		static void CleanUpUnused();

	private:

		static std::vector<std::function<void()>>& GetCleanupQueue();
		static std::vector<std::function<void()>>& GetCleanUnusedQueue();

		template<typename T>
		static std::unordered_map<UUID, Ref<T>>& GetCache()
		{
			static std::unordered_map<UUID, Ref<T>> cache;

			static bool isRegistered = false;
			if (!isRegistered)
			{
				GetCleanupQueue().push_back([]() {
					std::scoped_lock<std::mutex> lock(s_CacheMutex);
					GetCache<T>().clear();
					});
				isRegistered = true;

				GetCleanUnusedQueue().push_back([]() {
					std::scoped_lock<std::mutex> lock(s_CacheMutex);
					auto& currentCache = GetCache<T>();

					for (auto it = currentCache.begin(); it != currentCache.end(); )
					{
						if (it->second.use_count() == 1)
						{
							it = currentCache.erase(it);
						}
						else
						{
							++it;
						}
					}
					});
			}

			return cache;
		}

		static std::mutex s_CacheMutex;
	};
}




