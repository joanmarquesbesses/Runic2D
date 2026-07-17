#include "R2Dpch.h"
#include "AudioEngine.h"

#include <vector>

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

namespace Runic2D {

	static ma_engine s_AudioEngine;
	static ma_sound_group s_MusicGroup;
	static ma_sound_group s_SFXGroup;
	static bool s_Initialized = false;

	struct AudioSourceData {
		ma_sound Sound;
		bool InUse = false;
	};

	// Pool simple per a les fonts d'audio manejades
	static std::vector<AudioSourceData> s_Sources;

	static ma_sound_group* GetMAGroup(AudioGroup group)
	{
		return group == AudioGroup::Music ? &s_MusicGroup : &s_SFXGroup;
	}

	void AudioEngine::Init()
	{
		ma_result result = ma_engine_init(NULL, &s_AudioEngine);
		if (result != MA_SUCCESS) {
			R2D_CORE_ERROR("Failed to initialize Audio Engine!");
			return;
		}

		ma_sound_group_init(&s_AudioEngine, 0, NULL, &s_MusicGroup);
		ma_sound_group_init(&s_AudioEngine, 0, NULL, &s_SFXGroup);

		s_Sources.resize(1024); // Capacitat per a 1024 sons actius a la vegada

		s_Initialized = true;
		R2D_CORE_INFO("Audio Engine Initialized successfully!");
	}

	void AudioEngine::Shutdown()
	{
		if (!s_Initialized) return;

		for (auto& source : s_Sources)
		{
			if (source.InUse)
			{
				ma_sound_uninit(&source.Sound);
				source.InUse = false;
			}
		}

		ma_sound_group_uninit(&s_MusicGroup);
		ma_sound_group_uninit(&s_SFXGroup);

		ma_engine_uninit(&s_AudioEngine);
		s_Initialized = false;
	}

	void AudioEngine::Play(Ref<AudioClip> clip, AudioGroup group, float volume, float pitch)
	{
		if (!s_Initialized || !clip) return;

		ma_sound_group* pGroup = GetMAGroup(group);
		
		// Fire-and-forget
		// Utilitzem un engine intern que cacheja els arxius (ResourceManager integrat a miniaudio)
		// Aixo fa que no es llegeixi del disc cada cop.
		// miniaudio no permet passar pitch a play_sound directament, pero per defecte fa el play.
		// Com que es fire-and forget simple de miniaudio:
		ma_engine_play_sound(&s_AudioEngine, clip->GetFilepath().c_str(), pGroup);
	}

	void AudioEngine::SetGroupVolume(AudioGroup group, float volume)
	{
		if (!s_Initialized) return;
		ma_sound_group_set_volume(GetMAGroup(group), volume);
	}

	float AudioEngine::GetGroupVolume(AudioGroup group)
	{
		if (!s_Initialized) return 0.0f;
		return ma_sound_group_get_volume(GetMAGroup(group));
	}

	AudioEngine::AudioSourceHandle AudioEngine::CreateSource(Ref<AudioClip> clip, AudioGroup group, bool loop)
	{
		if (!s_Initialized || !clip) return 0;

		uint32_t handle = 0;
		bool found = false;
		for (uint32_t i = 1; i < s_Sources.size(); i++)
		{
			if (!s_Sources[i].InUse)
			{
				handle = i;
				found = true;
				break;
			}
		}

		if (!found)
		{
			R2D_CORE_ERROR("AudioEngine: S'ha arribat al limit maxim de fonts d'audio actius.");
			return 0;
		}

		AudioSourceData& source = s_Sources[handle];
		
		ma_uint32 flags = MA_SOUND_FLAG_NO_SPATIALIZATION; // Joc 2D, normalment no usem 3D encara

		ma_result result = ma_sound_init_from_file(&s_AudioEngine, clip->GetFilepath().c_str(), flags, GetMAGroup(group), NULL, &source.Sound);
		if (result != MA_SUCCESS)
		{
			R2D_CORE_ERROR("AudioEngine: Error instanciant la font de so per a {0}", clip->GetFilepath());
			return 0;
		}

		ma_sound_set_looping(&source.Sound, loop ? MA_TRUE : MA_FALSE);
		source.InUse = true;

		return handle;
	}

	void AudioEngine::DestroySource(AudioSourceHandle handle)
	{
		if (!s_Initialized || handle == 0 || handle >= s_Sources.size()) return;

		AudioSourceData& source = s_Sources[handle];
		if (source.InUse)
		{
			ma_sound_uninit(&source.Sound);
			source.InUse = false;
		}
	}

	void AudioEngine::PlaySource(AudioSourceHandle handle)
	{
		if (!s_Initialized || handle == 0 || handle >= s_Sources.size()) return;
		if (s_Sources[handle].InUse) ma_sound_start(&s_Sources[handle].Sound);
	}

	void AudioEngine::StopSource(AudioSourceHandle handle)
	{
		if (!s_Initialized || handle == 0 || handle >= s_Sources.size()) return;
		if (s_Sources[handle].InUse) ma_sound_stop(&s_Sources[handle].Sound);
	}

	void AudioEngine::PauseSource(AudioSourceHandle handle)
	{
		StopSource(handle);
	}

	void AudioEngine::SetSourceVolume(AudioSourceHandle handle, float volume)
	{
		if (!s_Initialized || handle == 0 || handle >= s_Sources.size()) return;
		if (s_Sources[handle].InUse) ma_sound_set_volume(&s_Sources[handle].Sound, volume);
	}

	void AudioEngine::SetSourcePitch(AudioSourceHandle handle, float pitch)
	{
		if (!s_Initialized || handle == 0 || handle >= s_Sources.size()) return;
		if (s_Sources[handle].InUse) ma_sound_set_pitch(&s_Sources[handle].Sound, pitch);
	}

	bool AudioEngine::IsSourcePlaying(AudioSourceHandle handle)
	{
		if (!s_Initialized || handle == 0 || handle >= s_Sources.size()) return false;
		if (s_Sources[handle].InUse) return ma_sound_is_playing(&s_Sources[handle].Sound) == MA_TRUE;
		return false;
	}

}
