#include "R2Dpch.h"
#include "TiledMapAsset.h"

#include "Runic2D/Utils/TiledParser.h"

namespace Runic2D
{

	Ref<TiledMapAsset> TiledMapAsset::Create(const std::string& filepath)
	{
		Ref<TiledMapAsset> mapAsset = CreateRef<TiledMapAsset>();

		TiledParser::LoadMap(filepath, nullptr, mapAsset.get());

		return mapAsset;
	}
} // namespace Runic2D

