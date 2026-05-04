#pragma once

#include <glm/glm.hpp>

namespace Runic2D {

    enum class EaseType {
        Linear,
        EaseInQuad,
        EaseOutQuad,
        EaseInOutQuad,
        EaseInCubic,
        EaseOutCubic,
        EaseInOutCubic,
        EaseInBack,
        EaseOutBack,
        EaseInOutBack,
        EaseOutBounce,
        EaseInElastic,
        EaseOutElastic
    };

    class Easing {
    public:
        static float Interpolate(float t, EaseType type) {
            switch (type) {
                case EaseType::Linear: return t;
                
                case EaseType::EaseInQuad: return t * t;
                case EaseType::EaseOutQuad: return t * (2.0f - t);
                case EaseType::EaseInOutQuad: return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;

                case EaseType::EaseInCubic: return t * t * t;
                case EaseType::EaseOutCubic: return (--t) * t * t + 1.0f;
                case EaseType::EaseInOutCubic: return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;

                case EaseType::EaseInBack: {
                    float s = 1.70158f;
                    return t * t * ((s + 1.0f) * t - s);
                }
                case EaseType::EaseOutBack: {
                    float s = 1.70158f;
                    return (--t) * t * ((s + 1.0f) * t + s) + 1.0f;
                }
                case EaseType::EaseInOutBack: {
                    float s = 1.70158f * 1.525f;
                    if ((t *= 2.0f) < 1.0f) return 0.5f * (t * t * ((s + 1.0f) * t - s));
                    return 0.5f * ((t -= 2.0f) * t * ((s + 1.0f) * t + s) + 2.0f);
                }

                case EaseType::EaseOutBounce: {
                    if (t < (1.0f / 2.75f)) return 7.5625f * t * t;
                    else if (t < (2.0f / 2.75f)) return 7.5625f * (t -= (1.5f / 2.75f)) * t + 0.75f;
                    else if (t < (2.5f / 2.75f)) return 7.5625f * (t -= (2.25f / 2.75f)) * t + 0.9375f;
                    else return 7.5625f * (t -= (2.625f / 2.75f)) * t + 0.984375f;
                }

                case EaseType::EaseOutElastic: {
                    float p = 0.3f;
                    return std::pow(2.0f, -10.0f * t) * std::sin((t - p / 4.0f) * (2.0f * glm::pi<float>()) / p) + 1.0f;
                }

                default: return t;
            }
        }
    };
}
