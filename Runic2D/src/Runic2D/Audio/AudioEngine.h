#pragma once

#include "Runic2D/Audio/AudioClip.h"
#include <string>

namespace Runic2D {

	enum class AudioGroup {
		Music = 0,
		SFX
	};

	class AudioEngine
	{
	public:
		static void Init();
		static void Shutdown();

		// Reproduccio Fire-and-forget (per explosions, trets, etc)
		static void Play(Ref<AudioClip> clip, AudioGroup group = AudioGroup::SFX, float volume = 1.0f, float pitch = 1.0f);

		// Gestio de volums globals
		static void SetGroupVolume(AudioGroup group, float volume);
		static float GetGroupVolume(AudioGroup group);

		// Reproduccio amb control continu (per Musica o Ambients que fan Loop)
		using AudioSourceHandle = uint32_t;
		static AudioSourceHandle CreateSource(Ref<AudioClip> clip, AudioGroup group, bool loop = false);
		static void DestroySource(AudioSourceHandle handle);
		
		static void PlaySource(AudioSourceHandle handle);
		static void StopSource(AudioSourceHandle handle);
		static void PauseSource(AudioSourceHandle handle);
		
		static void SetSourceVolume(AudioSourceHandle handle, float volume);
		static void SetSourcePitch(AudioSourceHandle handle, float pitch);
		static bool IsSourcePlaying(AudioSourceHandle handle);
	};

}
