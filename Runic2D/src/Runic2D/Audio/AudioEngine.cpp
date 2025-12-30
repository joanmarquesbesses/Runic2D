#include "R2Dpch.h"
#include "AudioEngine.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

namespace Runic2D {

	static ma_engine s_AudioEngine;
	static bool s_Initialized = false;

	void AudioEngine::Init()
	{
		ma_result result;

		result = ma_engine_init(NULL, &s_AudioEngine);

		if (result != MA_SUCCESS) {
			R2D_CORE_ERROR("Failed to initialize Audio Engine!");
			return;
		}

		s_Initialized = true;
		R2D_CORE_INFO("Audio Engine Initialized successfully!");
	}

	void AudioEngine::Shutdown()
	{
		if (s_Initialized)
		{
			ma_engine_uninit(&s_AudioEngine);
			s_Initialized = false;
		}
	}

	void AudioEngine::Play(const std::string& filepath)
	{
		if (!s_Initialized) return;

		if (!std::filesystem::exists(filepath))
		{
			R2D_CORE_ERROR("FILE NOT FOUND C++: {0}", filepath);
			return;
		}

		ma_result result = ma_engine_play_sound(&s_AudioEngine, filepath.c_str(), NULL);

		if (result != MA_SUCCESS)
		{
			R2D_CORE_ERROR("Audio Error: {0} | Path: {1}", (int)result, filepath);
		}
	}

}