#pragma once

#include "Runic2D/Audio/AudioClip.h"
#include "Runic2D/Core/Base/UUID.h"

namespace Runic2D {

	struct AudioSourceComponent
	{
		UUID AudioClipUUID = 0;
		Ref<AudioClip> Clip;

		float Volume = 1.0f;
		float Pitch = 1.0f;
		bool Looping = false;
		bool PlayOnAwake = true;

		uint32_t RuntimeHandle = 0; // No serialitzar
		
		bool RequestPlay = false;
		bool RequestStop = false;
		bool RequestPause = false;

		void Play() { RequestPlay = true; }
		void Stop() { RequestStop = true; }
		void Pause() { RequestPause = true; }

		AudioSourceComponent() = default;
		AudioSourceComponent(const AudioSourceComponent&) = default;
	};

}
