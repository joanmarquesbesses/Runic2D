#pragma once

#include "Runic2D/Core/Core.h"
#include <functional>

namespace Runic2D {

	class RUNIC_API JobSystem
	{
	public:
		static void Init();
		static void Shutdown();

		// Push a single job to the queue to be executed on a worker thread
		static void Execute(const std::function<void()>& job);

		// Wait until all submitted jobs are finished
		static void Wait();

		static uint32_t GetThreadCount();
	};

}
