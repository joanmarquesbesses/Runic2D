#pragma once

#include <functional>
#include <string>

namespace Runic2D {

	struct BackgroundTaskStats
	{
		uint32_t JobsInQueue = 0;
	};

	class RUNIC_API BackgroundTaskSystem
	{
	public:
		static void Init();
		static void Shutdown();

		// Llança una tasca al fil de fons. No bloqueja mai el JobSystem principal.
		static void Execute(const std::function<void()>& job);

		static BackgroundTaskStats GetStats();
	};

}
