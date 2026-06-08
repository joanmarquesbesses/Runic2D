#include "R2Dpch.h"
#include "TweenSystem.h"

#include "Runic2D/Scene/Scene.h"
#include "Runic2D/Scene/Component.h"

namespace Runic2D {
	void Runic2D::TweenSystem::OnUpdate(Timestep ts, Scene* scene)
	{
		std::vector<entt::entity> toRemove;
		std::vector<entt::entity> toDestroy;

		auto& registry = scene->GetEntityRegistry();

		auto view = registry.view<TweenComponent>();
		for (auto entityID : view)
		{
			Entity entity = { entityID, scene };
			auto& tc = entity.GetComponent<TweenComponent>();

			if (!tc.IsPlaying) continue;

			bool allFinished = true;
			for (auto& tween : tc.Tweens)
			{
				if (tween.Finished) continue;

				tween.TimeElapsed += ts;
				float t = glm::clamp(tween.TimeElapsed / tween.Duration, 0.0f, 1.0f);
				float easedT = Easing::Interpolate(t, tween.Easing);

				glm::vec4 currentVal = glm::mix(tween.StartValue, tween.EndValue, easedT);

				bool needsInvalidation = false;
				switch (tween.Target)
				{
				case TweenTarget::Position:
					if (entity.HasComponent<TransformComponent>()) entity.GetComponent<TransformComponent>().SetTranslation(glm::vec3(currentVal));
					if (entity.HasComponent<RectTransformComponent>()) entity.GetComponent<RectTransformComponent>().SetPosition(glm::vec2(currentVal));
					needsInvalidation = true;
					break;
				case TweenTarget::Scale:
					if (entity.HasComponent<TransformComponent>()) entity.GetComponent<TransformComponent>().SetScale(glm::vec3(currentVal));
					if (entity.HasComponent<RectTransformComponent>()) entity.GetComponent<RectTransformComponent>().SetScale(glm::vec2(currentVal));
					needsInvalidation = true;
					break;
				case TweenTarget::Rotation:
					if (entity.HasComponent<TransformComponent>()) entity.GetComponent<TransformComponent>().SetRotation(glm::vec3(currentVal));
					if (entity.HasComponent<RectTransformComponent>()) entity.GetComponent<RectTransformComponent>().SetRotation(currentVal.x);
					needsInvalidation = true;
					break;
				case TweenTarget::Color:
					if (entity.HasComponent<SpriteRendererComponent>()) entity.GetComponent<SpriteRendererComponent>().Color = currentVal;
					else if (entity.HasComponent<CircleRendererComponent>()) entity.GetComponent<CircleRendererComponent>().Color = currentVal;
					else if (entity.HasComponent<TextComponent>()) entity.GetComponent<TextComponent>().Color = currentVal;
					break;
				}

				if (needsInvalidation)
					entity.InvalidateTransform();

				if (t >= 1.0f)
				{
					if (tween.PingPong)
					{
						tween.TimeElapsed = 0.0f;
						std::swap(tween.StartValue, tween.EndValue);
						allFinished = false;
					}
					else
					{
						tween.Finished = true;
					}
				}
				else
				{
					allFinished = false;
				}
			}

			if (allFinished)
			{
				if (tc.OnComplete)
					tc.OnComplete(entity);

				if (tc.DestroyOnComplete)
					toDestroy.push_back(entityID);
				else
					toRemove.push_back(entityID);
			}
		}

		for (auto e : toRemove) registry.remove<TweenComponent>(e);
		for (auto e : toDestroy) registry.destroy(e);
	}
}
