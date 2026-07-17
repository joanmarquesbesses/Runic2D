#pragma once

#include "Runic2D/Math/Easing.h"
#include "Runic2D/Renderer/Animation2D.h"

namespace Runic2D {

	struct RUNIC_API AnimationProfile
	{
		std::string Name = "New Anim";

		UUID AtlasTextureUUID = 0;
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
