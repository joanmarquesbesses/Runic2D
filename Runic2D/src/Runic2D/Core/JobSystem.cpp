#include "R2Dpch.h"
#include "JobSystem.h"

#include <thread>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <atomic>

namespace Runic2D {

	namespace JobSystemImpl {
		static std::vector<std::thread> s_Threads;
		static std::queue<std::function<void()>> s_JobQueue;
		static std::mutex s_QueueMutex;
		static std::condition_variable s_WakeCondition;
		static std::atomic<uint32_t> s_JobsInFlight = 0;
		static bool s_Running = true;

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

					job = std::move(s_JobQueue.front());
					s_JobQueue.pop();
				}

				// Execute job
				if (job)
					job();
				
				s_JobsInFlight.fetch_sub(1);
			}
		}
	}

	void JobSystem::Init()
	{
		uint32_t numThreads = std::thread::hardware_concurrency();
		if (numThreads == 0) numThreads = 4; // Fallback
		
		// Leave one core for the main thread
		if (numThreads > 1) numThreads -= 1;

		JobSystemImpl::s_Running = true;
		JobSystemImpl::s_Threads.reserve(numThreads);
		
		for (uint32_t i = 0; i < numThreads; i++)
		{
			JobSystemImpl::s_Threads.emplace_back(JobSystemImpl::WorkerThread);
		}

		R2D_CORE_INFO("JobSystem Initialized with {0} worker threads.", numThreads);
	}

	void JobSystem::Shutdown()
	{
		JobSystemImpl::s_Running = false;
		JobSystemImpl::s_WakeCondition.notify_all();

		for (auto& thread : JobSystemImpl::s_Threads)
		{
			if (thread.joinable())
				thread.join();
		}
		JobSystemImpl::s_Threads.clear();
	}

	void JobSystem::Execute(const std::function<void()>& job)
	{
		JobSystemImpl::s_JobsInFlight.fetch_add(1);

		{
			std::unique_lock<std::mutex> lock(JobSystemImpl::s_QueueMutex);
			JobSystemImpl::s_JobQueue.push(job);
		}

		JobSystemImpl::s_WakeCondition.notify_one();
	}

	void JobSystem::Wait()
	{
		// Simple busy-wait/yield. For a more robust wait, use another condition_variable
		while (JobSystemImpl::s_JobsInFlight.load() > 0)
		{
			std::this_thread::yield();
		}
	}

	uint32_t JobSystem::GetThreadCount()
	{
		return (uint32_t)JobSystemImpl::s_Threads.size();
	}

}
