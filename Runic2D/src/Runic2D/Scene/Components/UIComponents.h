#pragma once

#include "Runic2D/Core/Core.h"
#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Runic2D {
	
	struct RUNIC_API RectTransformComponent
	{
	public:
		RectTransformComponent() = default;
		RectTransformComponent(const RectTransformComponent&) = default;

		// Getters
		const glm::vec2& GetPosition() const { return Position; }
		const glm::vec2& GetSize() const { return Size; }
		const glm::vec2& GetScale() const { return Scale; }
		float GetRotation() const { return Rotation; }
		
		const glm::vec2& GetAnchorMin() const { return AnchorMin; }
		const glm::vec2& GetAnchorMax() const { return AnchorMax; }
		const glm::vec2& GetPivot() const { return Pivot; }

		// Setters
		void SetPosition(const glm::vec2& pos) { Position = pos; m_IsDirty = true; }
		void SetSize(const glm::vec2& size) { Size = size; m_IsDirty = true; }
		void SetScale(const glm::vec2& scale) { Scale = scale; m_IsDirty = true; }
		void SetRotation(float rotation) { Rotation = rotation; m_IsDirty = true; }
		
		void SetAnchorMin(const glm::vec2& anchor) { AnchorMin = anchor; m_IsDirty = true; }
		void SetAnchorMax(const glm::vec2& anchor) { AnchorMax = anchor; m_IsDirty = true; }
		void SetPivot(const glm::vec2& pivot) { Pivot = pivot; m_IsDirty = true; }

		void CalculateTransforms(const glm::mat4& parentWorldTransform, const glm::vec2& parentSize, const glm::vec2& parentPivot,
			glm::mat4& outWorldTransform, glm::mat4& outMeshTransform) const
		{
			glm::vec2 anchorPos = -parentSize * parentPivot + parentSize * AnchorMin;
			glm::vec2 localPos = anchorPos + Position;

			glm::mat4 t = glm::translate(glm::mat4(1.0f), glm::vec3(localPos.x, localPos.y, 0.0f));
			glm::mat4 r = glm::rotate(glm::mat4(1.0f), Rotation, glm::vec3(0.0f, 0.0f, 1.0f));
			glm::mat4 s = glm::scale(glm::mat4(1.0f), glm::vec3(Scale.x, Scale.y, 1.0f));

			outWorldTransform = parentWorldTransform * t * r * s;

			glm::mat4 meshPivot = glm::translate(glm::mat4(1.0f), glm::vec3((0.5f - Pivot.x) * Size.x, (0.5f - Pivot.y) * Size.y, 0.0f));
			glm::mat4 meshScale = glm::scale(glm::mat4(1.0f), glm::vec3(Size.x, Size.y, 1.0f));

			outMeshTransform = outWorldTransform * meshPivot * meshScale;
		}

		mutable glm::mat4 ComputedMeshTransform = glm::mat4(1.0f);
		int ZIndex = 0;

	private:
		glm::vec2 Position = { 0.0f, 0.0f };
		glm::vec2 Size = { 100.0f, 100.0f };
		glm::vec2 AnchorMin = { 0.5f, 0.5f };
		glm::vec2 AnchorMax = { 0.5f, 0.5f };
		glm::vec2 Pivot = { 0.5f, 0.5f };
		float Rotation = 0.0f;
		glm::vec2 Scale = { 1.0f, 1.0f };

		mutable glm::mat4 WorldTransform = glm::mat4(1.0f);
		mutable bool m_IsDirty = true;

		friend class Scene;
		friend class UISystem;
		friend class Entity;
		friend class SceneSerializer;
	};

	struct RUNIC_API ButtonComponent
	{
		enum class State { Normal, Hovered, Pressed };

		glm::vec4 NormalColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 HoveredColor = { 0.85f, 0.85f, 0.85f, 1.0f };
		glm::vec4 PressedColor = { 0.65f, 0.65f, 0.65f, 1.0f };

		State CurrentState = State::Normal;

		std::function<void()> OnClick;
		std::function<void()> OnHover;
		std::function<void()> OnUnhover;

		ButtonComponent() = default;
		ButtonComponent(const ButtonComponent&) = default;
	};
}