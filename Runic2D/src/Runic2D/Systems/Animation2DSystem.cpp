#include "R2Dpch.h"
#include "Animation2DSystem.h"

#include "Runic2D/Scene/Component.h"
#include "Runic2D/Core/JobSystem.h"

namespace Runic2D {

	void Runic2D::Animation2DSystem::OnStart(Scene* scene)
	{
		scene->GetEntityRegistry().view<AnimationComponent>().each([=](auto entity, auto& anim)
			{
				anim.Animations.clear();

				for (auto& profile : anim.Profiles)
				{
					if (profile.AtlasTexture)
					{
						int numCols = (int)(profile.AtlasTexture->GetWidth() / profile.TileSize.x);
						int col = profile.StartFrame % numCols;
						int row = profile.StartFrame / numCols;

						float startX = col * profile.TileSize.x;
						float startY = row * profile.TileSize.y;

						Ref<Animation2D> animAsset = Animation2D::CreateFromAtlas(
							profile.AtlasTexture,
							profile.TileSize,
							{ startX, startY },
							profile.FrameCount,
							profile.FramesPerRow,
							profile.FrameTime
						);

						anim.Animations[profile.Name] = animAsset;
					}
				}

				if (!anim.Animations.empty())
				{
					if (!anim.CurrentStateName.empty() && anim.Animations.find(anim.CurrentStateName) != anim.Animations.end())
					{
						anim.CurrentAnimation = anim.Animations[anim.CurrentStateName];
					}
					else if (anim.Animations.find("Idle") != anim.Animations.end())
					{
						anim.CurrentAnimation = anim.Animations["Idle"];
						anim.CurrentStateName = "Idle";
					}
					else
					{
						anim.CurrentAnimation = anim.Animations.begin()->second;
						anim.CurrentStateName = anim.Animations.begin()->first;
					}

					anim.Playing = true;
				}
			});
	}

	void Runic2D::Animation2DSystem::OnUpdate(Timestep ts, Scene* scene)
	{
		if (scene->IsPaused()) return;

		struct AnimEntry
		{
			AnimationComponent* Anim;
			SpriteRendererComponent* Sprite;
		};

		auto view = scene->GetEntityRegistry().view<AnimationComponent, SpriteRendererComponent>();
		std::vector<AnimEntry> entries;
		entries.reserve(view.size_hint());

		view.each([&](auto entityID, auto& anim, auto& sprite)
			{
				entries.emplace_back(&anim, &sprite);
			});

		if (entries.empty()) return;

		for (auto& entry : entries)
		{
			auto& anim = *entry.Anim;
			if (anim.Animations.empty() && !anim.Profiles.empty())
			{
				for (auto& profile : anim.Profiles)
				{
					if (profile.AtlasTexture)
					{
						int numCols = (int)(profile.AtlasTexture->GetWidth() / profile.TileSize.x);
						if (numCols <= 0) numCols = 1;
						int col = profile.StartFrame % numCols;
						int row = profile.StartFrame / numCols;
						float startX = (float)col * profile.TileSize.x;
						float startY = (float)row * profile.TileSize.y;

						Ref<Animation2D> animAsset = Animation2D::CreateFromAtlas(
							profile.AtlasTexture, profile.TileSize, { startX, startY },
							profile.FrameCount, profile.FramesPerRow, profile.FrameTime
						);
						anim.Animations[profile.Name] = animAsset;
					}
				}

				if (!anim.Animations.empty()) {
					auto it = anim.Animations.find(anim.CurrentStateName);
					if (it != anim.Animations.end()) {
						anim.CurrentAnimation = it->second;
					}
					else {
						anim.CurrentAnimation = anim.Animations.begin()->second;
						anim.CurrentStateName = anim.Animations.begin()->first;
					}
				}
			}
		}

		const uint32_t count = (uint32_t)entries.size();
		const uint32_t groupSize = 64;

		auto stats = JobSystem::Dispatch(count, groupSize, [&entries, ts](uint32_t start, uint32_t end)
			{
				for (uint32_t i = start; i < end; i++)
				{
					auto& anim = *entries[i].Anim;
					auto& sprite = *entries[i].Sprite;

					if (!anim.CurrentAnimation) continue;

					if (anim.Playing) {
						anim.TimeAccumulator += ts;
						while (anim.TimeAccumulator >= anim.CurrentAnimation->GetFrameTime())
						{
							anim.TimeAccumulator -= anim.CurrentAnimation->GetFrameTime();
							anim.CurrentFrameIndex++;

							if (anim.CurrentFrameIndex >= anim.CurrentAnimation->GetFrameCount())
							{
								if (anim.Loop) anim.CurrentFrameIndex = 0;
								else {
									anim.CurrentFrameIndex = anim.CurrentAnimation->GetFrameCount() - 1;
									anim.Playing = false;
								}
							}
						}
						sprite.SubTexture = anim.CurrentAnimation->GetFrame(anim.CurrentFrameIndex);
					}
					else {
						if (anim.CurrentFrameIndex >= anim.CurrentAnimation->GetFrameCount()) {
							anim.CurrentFrameIndex = 0;
						}
						sprite.SubTexture = anim.CurrentAnimation->GetFrame(anim.CurrentFrameIndex);
					}
				}
			});

		if (stats.GroupsDispatched > 0)
		{
			JobSystem::Wait();
		}
	}
}