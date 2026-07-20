#pragma once

#include "Runic2D/Core/Base/Core.h"
#include "Runic2D/Assets/Asset.h"
#include <filesystem>

namespace Runic2D {

	class RUNIC_API Prefab : public Asset
	{
	public:
		Prefab(const std::filesystem::path& filepath);
		~Prefab();

		const std::filesystem::path& GetFilepath() const { return m_Filepath; }

		static Ref<Prefab> Create(const std::filesystem::path& filepath);

	private:
		std::filesystem::path m_Filepath;
	};

}