#pragma once

#include "Runic2D/Core/Core.h"
#include <functional>
#include <cstdint>

namespace Runic2D {

	struct DispatchStats
	{
		uint32_t GroupsDispatched = 0; // 0 = ran inline on calling thread, >1 = used worker threads
	};

	class RUNIC_API JobSystem
	{
	public:
		static void Init();
		static void Shutdown();

		// Push a single job to the queue to be executed on a worker thread
		static void Execute(const std::function<void()>& job);

		// Execute a job across multiple threads by splitting dataCount into chunks of groupSize.
		// If only 1 group is needed, runs inline on the calling thread (no queue overhead).
		// Returns stats about how many groups were actually dispatched to threads.
		static DispatchStats Dispatch(uint32_t dataCount, uint32_t groupSize, const std::function<void(uint32_t start, uint32_t end)>& task);

		// Wait until all submitted jobs are finished
		static void Wait();

		static uint32_t GetThreadCount();
		static uint32_t GetThreadIndex();
	};

}
