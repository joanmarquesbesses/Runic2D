#pragma once

#include "Runic2D/Assets/Asset.h"
#include <string>

namespace Runic2D {

	class AudioClip : public Asset
	{
	public:
		static Ref<AudioClip> Create(const std::string& filepath);

		AudioClip(const std::string& filepath);
		virtual ~AudioClip() = default;

		const std::string& GetFilepath() const { return m_Filepath; }

	private:
		std::string m_Filepath;
	};

}
