#pragma once

#include "Runic2D/Core/Base/Core.h"
#include "Runic2D/Scene/ScriptableEntity.h"

namespace Runic2D {

	struct RUNIC_API FlockingComponent
	{
		float NeighborRadius = 3.0f;     // A quina distància un altre monstre es considera "veí"
		// Força i prioritat de cada regla
		float SeparationWeight = 1.5f;   // Normalment la separació és la més alta per no fer "pilons"
		float AlignmentWeight = 1.0f;
		float CohesionWeight = 1.0f;

		float TargetWeight = 2.0f;       // Les ganes de matar al jugador pesen més que els companys
	};

	struct RUNIC_API PathfindingComponent
	{
		UUID TargetEntity = 0;
		bool Enabled = false;
		std::vector<glm::vec2> Path;
		size_t CurrentWaypointIndex = 0;
		float RepathTimer = 0.0f;
		float RepathInterval = 0.5f;
		PathfindingComponent() = default;
		PathfindingComponent(const PathfindingComponent&) = default;
	};

}
