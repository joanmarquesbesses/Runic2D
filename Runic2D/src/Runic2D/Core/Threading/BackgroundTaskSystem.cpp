#include "R2Dpch.h"
#include "Runic2D/Core/Threading/BackgroundTaskSystem.h"

#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <vector>
#include <atomic>

namespace Runic2D {

	namespace BackgroundImpl {
		static std::vector<std::thread> s_Threads;
		static std::queue<std::function<void()>> s_JobQueue;
		static std::mutex s_QueueMutex;
		static std::condition_variable s_WakeCondition;
		static bool s_Running = false;
		static bool s_Enabled = true;

		static void WorkerThread()
		{
			while (s_Running)
			{
				std::function<void()> job;

				{
					std::unique_lock<std::mutex> lock(s_QueueMutex);
					s_WakeCondition.wait(lock, [] { return !s_JobQueue.empty() || !s_Running; });

					if (!s_Running && s_JobQueue.empty())
						return;

					if (!s_JobQueue.empty())
					{
						job = std::move(s_JobQueue.front());
						s_JobQueue.pop();
					}
				}

				if (job)
				{
					try {
						job();
					}
					catch (const std::exception& e) {
					}
				}
			}
		}
	}

	void BackgroundTaskSystem::Init()
	{
		if (BackgroundImpl::s_Running) return;

		BackgroundImpl::s_Running = true;

		// Amb 4 fils podrem carregar diverses textures a la vegada mentre es llegeix el YAML
		uint32_t numThreads = 4; 
		
		for (uint32_t i = 0; i < numThreads; i++)
		{
			BackgroundImpl::s_Threads.emplace_back(BackgroundImpl::WorkerThread);
		}
	}

	void BackgroundTaskSystem::Shutdown()
	{
		{
			std::unique_lock<std::mutex> lock(BackgroundImpl::s_QueueMutex);
			BackgroundImpl::s_Running = false;
		}
		BackgroundImpl::s_WakeCondition.notify_all();

		for (auto& thread : BackgroundImpl::s_Threads)
		{
			if (thread.joinable())
				thread.join();
		}
		BackgroundImpl::s_Threads.clear();
	}

	void BackgroundTaskSystem::SetEnabled(bool enabled)
	{
		BackgroundImpl::s_Enabled = enabled;
	}

	bool BackgroundTaskSystem::IsEnabled()
	{
		return BackgroundImpl::s_Enabled;
	}

	void BackgroundTaskSystem::Execute(const std::function<void()>& job)
	{
		if (!BackgroundImpl::s_Enabled)
		{
			if (job) job();
			return;
		}

		{
			std::unique_lock<std::mutex> lock(BackgroundImpl::s_QueueMutex);
			BackgroundImpl::s_JobQueue.push(job);
		}
		BackgroundImpl::s_WakeCondition.notify_one();
	}

	BackgroundTaskStats BackgroundTaskSystem::GetStats()
	{
		BackgroundTaskStats stats;
		std::unique_lock<std::mutex> lock(BackgroundImpl::s_QueueMutex);
		stats.JobsInQueue = (uint32_t)BackgroundImpl::s_JobQueue.size();
		return stats;
	}

}

