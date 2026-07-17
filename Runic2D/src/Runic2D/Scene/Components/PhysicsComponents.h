#pragma once

#include "Runic2D/Core/Base/Core.h"

#include <glm/glm.hpp>
#include <box2d/types.h>

namespace Runic2D {

	struct RUNIC_API Rigidbody2DComponent
	{
		enum class BodyType { Static = 0, Dynamic, Kinematic };
		BodyType Type = BodyType::Static;
		bool FixedRotation = false;

		float GravityScale = 1.0f;

		b2BodyId RuntimeBody = b2_nullBodyId;

		glm::vec2 PreviousTranslation = { 0.0f, 0.0f };
		float PreviousRotation = 0.0f;
		bool InterpolationInitialized = false; // Prevents first-frame visual glitch

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
	};

	struct RUNIC_API BoxCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		glm::vec2 Size = { 1.0f, 1.0f };

		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		uint32_t CategoryBits = 0x0001;
		uint32_t MaskBits = 0xFFFFFFFF;
		int32_t GroupIndex = 0;

		bool IsSensor = false;
		bool EnableContactEvents = true;
		bool EnableSensorEvents = false;

		b2ShapeId RuntimeShape = b2_nullShapeId;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};

	struct RUNIC_API CircleCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		float Radius = 0.5f;

		// TODO(Yan): move into physics material in the future maybe
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		uint32_t CategoryBits = 0x0001;
		uint32_t MaskBits = 0xFFFFFFFF;
		int32_t GroupIndex = 0;

		bool IsSensor = false;
		bool EnableContactEvents = true;
		bool EnableSensorEvents = false;

		b2ShapeId RuntimeShape = b2_nullShapeId;

		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
	};
}
