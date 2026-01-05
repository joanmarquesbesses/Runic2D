#pragma once

#include <random>

namespace Runic2D {

	class Random
	{
	public:
		static void Init();

		static float Float();

		static float Range(float min, float max);

	private:
		static std::mt19937 s_RandomEngine;
		static std::uniform_int_distribution<std::mt19937::result_type> s_Distribution;
	};

}
