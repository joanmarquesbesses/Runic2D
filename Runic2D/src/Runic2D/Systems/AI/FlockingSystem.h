#pragma once

#include "Runic2D/Systems/System.h"
#include "Runic2D/Math/SpatialHash.h"

namespace Runic2D {

	class RUNIC_API FlockingSystem : public System
	{
	public:
		virtual void OnUpdate(Timestep ts, Scene* scene) override;

		const SpatialHash& GetSpatialHash() const { return m_SpatialHash; }
	private:
		SpatialHash m_SpatialHash{ 2.0f };
	};

}