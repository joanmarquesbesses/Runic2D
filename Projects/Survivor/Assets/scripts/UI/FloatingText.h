#pragma once
#include "Runic2D.h"

using namespace Runic2D;

namespace Survivor {

    class FloatingText : public ScriptableEntity {
    public:
        float m_LifeTime = 1.0f;
        float m_Speed = 1.5f;
        float m_FadeSpeed = 2.0f;

        void OnUpdate(Timestep ts) override {
            m_LifeTime -= ts;

            auto& transform = GetComponent<TransformComponent>();
            glm::vec3 translation = transform.GetTranslation();
            translation.y += m_Speed * ts;
            transform.SetTranslation(translation);

            if (m_LifeTime < 0.5f) {
                auto& textComp = GetComponent<TextComponent>();

                textComp.Color.a -= m_FadeSpeed * ts;
                if (textComp.Color.a < 0.0f) textComp.Color.a = 0.0f;
            }

            if (m_LifeTime <= 0.0f) {
                Destroy();
            }
        }
    };

}