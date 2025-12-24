#include "R2Dpch.h"
#include "ResourceManager.h"

namespace Runic2D {

	std::vector<std::function<void()>>& ResourceManager::GetCleanupQueue()
	{
		static std::vector<std::function<void()>> queue;
		return queue;
	}

	std::vector<std::function<void()>>& ResourceManager::GetCleanUnusedQueue()
	{
		static std::vector<std::function<void()>> queue;
		return queue;
	}

	void ResourceManager::Clear()
	{
		for (auto& clearFunc : GetCleanupQueue())
		{
			clearFunc();
		}

		R2D_CORE_INFO("ResourceManager: All caches cleared.");
	}

    void ResourceManager::CleanUpUnused()
    {
		for (auto& func : GetCleanUnusedQueue())
		{
			func();
		}
    }

}