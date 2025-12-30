#pragma once

#include <string>

namespace Runic2D {

	class AudioEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static void Play(const std::string& filepath);
	};

}