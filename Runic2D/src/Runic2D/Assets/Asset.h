#pragma once

#include "Runic2D/Core/UUID.h"

namespace Runic2D {

	using AssetHandle = UUID;

	class RUNIC_API Asset
	{
	public:
		AssetHandle Handle = 0; // 0 = no valid

		virtual ~Asset() {};
	};

}