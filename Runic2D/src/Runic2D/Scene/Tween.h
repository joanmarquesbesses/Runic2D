#pragma once

#include "Runic2D/Scene/Entity.h"
#include "Runic2D/Scene/Components/CoreComponents.h"
#include "Runic2D/Scene/Components/RenderComponents.h"
#include "Runic2D/Scene/Components/MotionComponents.h"
#include "Runic2D/Scene/Components/UIComponents.h"

#include <glm/glm.hpp>

namespace Runic2D {

	class Tween {
	public:
		static TweenData& To(Entity entity, TweenTarget target, glm::vec4 endValue, float duration, EaseType easing = EaseType::Linear)
		{
			if (!entity.HasComponent<TweenComponent>())
				entity.AddComponent<TweenComponent>();

			auto& tc = entity.GetComponent<TweenComponent>();

			TweenData data;
			data.Target = target;
			data.Easing = easing;
			data.Duration = duration;
			data.EndValue = endValue;
			data.StartValue = GetCurrentValue(entity, target);

			tc.Tweens.push_back(data);
			return tc.Tweens.back();
		}

				static void Clear(Entity entity)
		{
			if (entity.HasComponent<TweenComponent>())
				entity.RemoveComponent<TweenComponent>();
		}

		static void ClearTarget(Entity entity, TweenTarget target)
		{
			if (!entity.HasComponent<TweenComponent>()) return;

			auto& tc = entity.GetComponent<TweenComponent>();
			for (auto it = tc.Tweens.begin(); it != tc.Tweens.end(); )
			{
				if (it->Target == target)
					it = tc.Tweens.erase(it);
				else
					++it;
			}
		}

	private:
		static glm::vec4 GetCurrentValue(Entity entity, TweenTarget target)
		{
			switch (target)
			{
			case TweenTarget::Position:
				if (entity.HasComponent<TransformComponent>())
					return glm::vec4(entity.GetComponent<TransformComponent>().GetTranslation(), 0.0f);
				if (entity.HasComponent<RectTransformComponent>())
					return glm::vec4(entity.GetComponent<RectTransformComponent>().GetPosition(), 0.0f, 0.0f);
				break;
			case TweenTarget::Scale:
				if (entity.HasComponent<TransformComponent>())
					return glm::vec4(entity.GetComponent<TransformComponent>().GetScale(), 0.0f);
				if (entity.HasComponent<RectTransformComponent>())
					return glm::vec4(entity.GetComponent<RectTransformComponent>().GetScale(), 0.0f, 0.0f);
				break;
			case TweenTarget::Rotation:
				if (entity.HasComponent<TransformComponent>())
					return glm::vec4(entity.GetComponent<TransformComponent>().GetRotation(), 0.0f);
				if (entity.HasComponent<RectTransformComponent>())
					return glm::vec4(entity.GetComponent<RectTransformComponent>().GetRotation(), 0.0f, 0.0f, 0.0f);
				break;
			case TweenTarget::Color:
				if (entity.HasComponent<SpriteRendererComponent>())
					return entity.GetComponent<SpriteRendererComponent>().Color;
				if (entity.HasComponent<CircleRendererComponent>())
					return entity.GetComponent<CircleRendererComponent>().Color;
				if (entity.HasComponent<TextComponent>())
					return entity.GetComponent<TextComponent>().Color;
				break;
			}
			return glm::vec4(0.0f);
		}
	};

}

