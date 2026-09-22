#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Runic2D/Core/Base/UUID.h"
#include <entt.hpp>

namespace Runic2D {

	struct RUNIC_API IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		// Constructor que accepta un UUID concret
		IDComponent(const UUID& uuid) : ID(uuid) {}
	};

	struct RUNIC_API TagComponent {
		std::string Tag;
		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {
		}
		operator std::string& () { return Tag; }
		operator const std::string& () const { return Tag; }
	};

	struct RUNIC_API TransformComponent {
	public:
		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation)
			: Translation(translation), IsDirty(true) {
		}
		TransformComponent(const glm::mat4& transform)
			: Transform(transform), IsDirty(true) {
		}

		const glm::vec3& GetTranslation() const { return Translation; }
		const glm::vec3& GetRotation() const { return Rotation; }
		const glm::vec3& GetScale() const { return Scale; }

		void SetTranslation(const glm::vec3& translation) { Translation = translation; IsDirty = true; }
		void SetRotation(const glm::vec3& rotation) { Rotation = rotation; IsDirty = true; }
		void SetScale(const glm::vec3& scale) { Scale = scale; IsDirty = true; }

		void SetDirty() { IsDirty = true; }
		bool GetDirty() const { return IsDirty; }
		void ClearDirty() { IsDirty = false; }

		operator glm::mat4& () { return const_cast<glm::mat4&>(GetTransform()); }
		operator const glm::mat4& () const { return GetTransform(); }

		const glm::mat4& GetTransform() const {
			if (IsDirty) {
				glm::mat4 rot = glm::toMat4(glm::quat(Rotation));
				Transform = glm::translate(glm::mat4(1.0f), Translation)
					* rot
					* glm::scale(glm::mat4(1.0f), Scale);

				// IsDirty cleared by Scene
			}
			return Transform;
		}

		const glm::mat4& GetWorldTransform() const { return WorldTransform; }
		void SetWorldTransform(const glm::mat4& transform) { WorldTransform = transform; }

	private:
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		mutable glm::mat4 Transform = glm::mat4(1.0f);
		mutable glm::mat4 WorldTransform = glm::mat4(1.0f);
		mutable bool IsDirty = true;
	};

	struct RUNIC_API RelationshipComponent
	{
		entt::entity Parent = entt::null;
		entt::entity FirstChild = entt::null;
		entt::entity NextSibling = entt::null;
		entt::entity PrevSibling = entt::null;

		std::size_t ChildrenCount = 0;

		RelationshipComponent() = default;
		RelationshipComponent(const RelationshipComponent&) = default;
	};
}
