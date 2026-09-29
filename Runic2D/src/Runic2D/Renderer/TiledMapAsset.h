#pragma once

#include "Runic2D/Assets/Asset.h"
#include "Runic2D/Renderer/SubTexture2D.h"

#include <unordered_map>
#include <vector>
#include <string>

namespace Runic2D {

	struct RUNIC_API TileLayer {
		std::string Name;
		std::vector<int> Data;
	};

	struct RUNIC_API TiledColliderData {
		float X, Y;
		float Width, Height;
	};


	class RUNIC_API TiledMapAsset : public Asset
	{
	public:
		int MapWidth = 0;
		int MapHeight = 0;


		std::vector<TileLayer> TileLayers;
		std::vector<TiledColliderData> Colliders;

		AssetHandle TilesetTextureHandle = 0;
		int TilesetColumns = 1;
		int TileWidth = 32;
		int TileHeight = 32;

		static Ref<TiledMapAsset> Create(const std::string& filepath);

		std::unordered_map<int, Ref<SubTexture2D>> TileCache;
	};
}