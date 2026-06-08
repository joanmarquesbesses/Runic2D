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
#include "Runic2D/Math/Easing.h"
#include "Runic2D/Renderer/Animation2D.h"

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

	struct RUNIC_API SpriteRendererComponent {
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Texture2D> Texture;
		Ref<SubTexture2D> SubTexture;
		float TilingFactor = 1.0f;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color)
			: Color(color) {}
	};

	struct RUNIC_API CircleRendererComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		float Thickness = 1.0f;
		float Fade = 0.005f;

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;
	};

	struct RUNIC_API CameraComponent {
		SceneCamera Camera;
		bool Primary = true; //TODO: think about moving to scene
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	struct RUNIC_API NativeScriptComponent {
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

	struct RUNIC_API TextComponent
	{
		enum class Alignment { Left = 0, Center, Right };

	private:
		std::string m_TextString = "";
		mutable float m_CachedWidth = 0.0f;
		mutable bool m_IsDirty = true;

	public:
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
		Ref<Font> FontAsset = Font::GetDefault();
		float Kerning = 0.0f;
		float LineSpacing = 0.0f;
		bool Visible = true;

		Alignment TextAlignment = Alignment::Center;

		TextComponent() = default;
		TextComponent(const TextComponent&) = default;
		TextComponent(const std::string& text) { SetText(text); }

		void SetText(const std::string& text)
		{
			if (m_TextString != text)
			{
				m_TextString = text;
				m_IsDirty = true;
			}
		}

		const std::string& GetText() const { return m_TextString; }

		float GetTextWidth() const
		{
			if (m_IsDirty && FontAsset)
			{
				m_CachedWidth = FontAsset->GetStringWidth(m_TextString, Kerning);
			}
			return m_CachedWidth;
		}

		void MarkDirty() { m_IsDirty = true; }
	};

	struct RUNIC_API AnimationProfile
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

	struct RUNIC_API AnimationComponent
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

		void Play(const std::string& name)
		{
			if (CurrentStateName == name) return;
			auto it = Animations.find(name);
			if (it != Animations.end())
			{
				CurrentAnimation = it->second;
				CurrentStateName = name;
				CurrentFrameIndex = 0;
				TimeAccumulator = 0.0f;
				Playing = true;
				for (auto& profile : Profiles)
				{
					if (profile.Name == name)
					{
						Loop = profile.Loop;
						break;
					}
				}
			}
		}
	};

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

	enum class TweenTarget { Position, Scale, Rotation, Color };

	struct TweenData {
		TweenTarget Target;
		EaseType Easing;
		glm::vec4 StartValue;
		glm::vec4 EndValue;
		float Duration = 1.0f;
		float TimeElapsed = 0.0f;
		bool PingPong = false;
		bool Reverse = false;
		bool Finished = false;
	};

	struct RUNIC_API TweenComponent {
		std::vector<TweenData> Tweens;
		bool IsPlaying = true;
		bool DestroyOnComplete = false;
		std::function<void(Entity)> OnComplete = nullptr;

		TweenComponent() = default;
		TweenComponent(const TweenComponent&) = default;
	};
}