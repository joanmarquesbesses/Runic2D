#include "R2Dpch.h"
#include "AudioClip.h"

namespace Runic2D {

	Ref<AudioClip> AudioClip::Create(const std::string& filepath)
	{
		return CreateRef<AudioClip>(filepath);
	}

	AudioClip::AudioClip(const std::string& filepath)
		: m_Filepath(filepath)
	{
	}

}
