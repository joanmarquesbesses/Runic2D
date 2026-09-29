#pragma once

#include "Runic2D/Core/Base/Core.h"
#include "Runic2D/Scene/Scene.h"
#include "Runic2D/Renderer/TiledMapAsset.h"

#include <string>

namespace Runic2D {

	class RUNIC_API TiledParser
	{
	public:
		static bool LoadMap(const std::string& filepath, Ref<Scene> scene, TiledMapAsset* outComponent);
	};

}