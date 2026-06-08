#pragma once
#include "System.h"

namespace Runic2D {
	class RUNIC_API Render2DSystem : public System
	{
	public:
		virtual ~Render2DSystem() override {};
		virtual void OnRender(Scene* scene) override;
	};
}

