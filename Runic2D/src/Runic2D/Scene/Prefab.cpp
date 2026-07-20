#include "R2Dpch.h"
#include "Prefab.h"

namespace Runic2D {

	Prefab::Prefab(const std::filesystem::path& filepath)
		: m_Filepath(filepath)
	{
	}

	Prefab::~Prefab()
	{
	}

	Ref<Prefab> Prefab::Create(const std::filesystem::path& filepath)
	{
		return CreateRef<Prefab>(filepath);
	}

}