#pragma once

#include "Runic2D/Core/Core.h"
#include <unordered_map>
#include <string>
#include <filesystem>
#include <memory>
#include <functional>
#include <vector>

namespace Runic2D {

	class ResourceManager
	{
	public:

		template<typename T>
		static Ref<T> Get(const std::filesystem::path& path)
		{
			auto& cache = GetCache<T>();

			std::string pathString = path.string();
			std::replace(pathString.begin(), pathString.end(), '\\', '/');

			if (cache.find(pathString) != cache.end())
			{
				return cache[pathString];
			}

			if (!std::filesystem::exists(path))
			{
				R2D_CORE_WARN("ResourceManager: Asset not found at {0}", pathString);
				return nullptr;
			}

			Ref<T> newAsset = T::Create(pathString);

			cache[pathString] = newAsset;
			return newAsset;
		}

		static void Clear();

		static void CleanUpUnused();

	private:

		static std::vector<std::function<void()>>& GetCleanupQueue();

		static std::vector<std::function<void()>>& GetCleanUnusedQueue();

		template<typename T>
		static std::unordered_map<std::string, Ref<T>>& GetCache()
		{
			static std::unordered_map<std::string, Ref<T>> cache;

			static bool isRegistered = false;
			if (!isRegistered)
			{
				GetCleanupQueue().push_back([]() {
					GetCache<T>().clear();
					});
				isRegistered = true;

				GetCleanUnusedQueue().push_back([]() {
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
	};
}