#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <box2d/types.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "Runic2D/Core/UUID.h"
#include "SceneCamera.h"
#include "ScriptableEntity.h"
#include "Runic2D/Renderer/Texture.h"
#include "Runic2D/Renderer/SubTexture2D.h"
#include "Runic2D/Renderer/Font.h"
#include "Runic2D/Renderer/Animation2D.h"

namespace Runic2D {

	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
		// Constructor que accepta un UUID concret
		IDComponent(const UUID& uuid) : ID(uuid) {}
	};

	struct TagComponent {
		std::string Tag;
		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {
		}
		operator std::string& () { return Tag; }
		operator const std::string& () const { return Tag; }
	};

	struct TransformComponent {
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		mutable glm::mat4 Transform = glm::mat4(1.0f);
		mutable bool IsDirty = true;

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::mat4& transform)
			: Transform(transform), IsDirty(true) {
		}
		TransformComponent(const glm::mat4&& transform)
			: Transform(transform), IsDirty(true) {
		}

		void SetTranslation(const glm::vec3& translation) {
			Translation = translation;
			IsDirty = true;
		}

		void SetRotation(const glm::vec3& rotation) {
			Rotation = rotation;
			IsDirty = true;
		}

		void SetScale(const glm::vec3& scale) {
			Scale = scale;
			IsDirty = true;
		}

		operator glm::mat4& () { return Transform; }
		operator const glm::mat4& () const { return Transform; }

		const glm::mat4& GetTransform() const {
			if (IsDirty) {
				glm::mat4 rot = glm::toMat4(glm::quat(Rotation));
				Transform = glm::translate(glm::mat4(1.0f), Translation)
					* rot
					* glm::scale(glm::mat4(1.0f), Scale);

				IsDirty = false;
			}
			return Transform;
		}
	};

	struct SpriteRendererComponent {
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture;
		Ref<SubTexture2D> SubTexture;
		float TilingFactor = 1.0f;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color)
			: Color(color) {}
	};

	struct CircleRendererComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		float Thickness = 1.0f;
		float Fade = 0.005f;

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;
	};

	struct CameraComponent {
		SceneCamera Camera;
		bool Primary = true; //TODO: think about moving to scene
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	struct NativeScriptComponent {
		ScriptableEntity* Instance = nullptr;
		
		ScriptableEntity* (*InstantiateScript)();
		void (*DestroyScript)(NativeScriptComponent*);

		std::string ClassName = "None";

		template<typename T>
		void Bind() {
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}

	};

	struct RelationshipComponent
	{
		entt::entity Parent = entt::null;
		entt::entity FirstChild = entt::null;
		entt::entity NextSibling = entt::null;
		entt::entity PrevSibling = entt::null;

		std::size_t ChildrenCount = 0;

		RelationshipComponent() = default;
		RelationshipComponent(const RelationshipComponent&) = default;
	};

	struct Rigidbody2DComponent
	{
		enum class BodyType { Static = 0, Dynamic, Kinematic };
		BodyType Type = BodyType::Static;
		bool FixedRotation = false;

		float GravityScale = 1.0f;

		b2BodyId RuntimeBody = b2_nullBodyId;

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
	};

	struct BoxCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		glm::vec2 Size = { 1.0f, 1.0f };

		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		bool IsSensor = false;

		bool EnableContactEvents = true;
		bool EnableSensorEvents = false;

		b2ShapeId RuntimeShape = b2_nullShapeId;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};

	struct CircleCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		float Radius = 0.5f;

		// TODO(Yan): move into physics material in the future maybe
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		bool IsSensor = false;

		bool EnableContactEvents = true;
		bool EnableSensorEvents = false;

		b2ShapeId RuntimeShape = b2_nullShapeId;

		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
	};

	struct TextComponent
	{
		std::string TextString = "";
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Font> FontAsset = Font::GetDefault();
		float Kerning = 0.0f;
		float LineSpacing = 0.0f;

		TextComponent() = default;
		TextComponent(const TextComponent&) = default;
		TextComponent(const std::string& text) : TextString(text) {}
	};

	struct AnimationProfile
	{
		std::string Name = "New Anim";

		Ref<Texture2D> AtlasTexture = nullptr;
		std::string TexturePath = "";

		glm::vec2 TileSize = { 32.0f, 32.0f };

		int StartFrame = 0; 
		int FrameCount = 1; 
		int FramesPerRow = 0;
		float FrameTime = 0.1f; 

		bool Loop = true;
	};

	struct AnimationComponent
	{
		std::vector<AnimationProfile> Profiles;

		std::map<std::string, Ref<Animation2D>> Animations;
		Ref<Animation2D> CurrentAnimation = nullptr;
		std::string CurrentStateName = "";

		float TimeAccumulator = 0.0f; 
		uint32_t CurrentFrameIndex = 0;
		bool Loop = true; 
		bool Playing = true;

		AnimationComponent() = default;
		AnimationComponent(const AnimationComponent&) = default;

		bool IsFinished() const
		{
			if (!CurrentAnimation) return true;

			if (Loop) return false;

			return CurrentFrameIndex >= (CurrentAnimation->GetFrameCount() - 1);
		}
	};

	struct ProjectileComponent
	{
		float Speed = 10.0f;
		float LifeTime = 2.0f;
		float Damage = 10.0f;

		glm::vec2 Direction = { 1.0f, 0.0f };

		ProjectileComponent() = default;
		ProjectileComponent(const ProjectileComponent&) = default;
	};
}