#pragma once

#include "System.h"
#include "Runic2D/Renderer/Texture.h"

#include <vector>
#include <glm/glm.hpp>

namespace Runic2D {

    struct ParticleProps
    {
        glm::vec3 Position;
        glm::vec2 Velocity, VelocityVariation;
        glm::vec4 ColorBegin, ColorEnd;
        float SizeBegin, SizeEnd, SizeVariation;
        float LifeTime = 1.0f;
		Ref<Texture2D> Texture = nullptr;
    };

    class RUNIC_API ParticleSystem : public System
    {
    public:
        ParticleSystem(uint32_t maxParticles = 10000);
        ~ParticleSystem() {}

        virtual void OnUpdate(Timestep ts, Scene* scene) override;
        virtual void OnRender(Scene* scene) override;

        void Emit(const ParticleProps& particleProps);

        int GetActiveParticleCount() const;

    private:
        struct Particle
        {
            glm::vec3 Position;
            glm::vec2 Velocity;
            glm::vec4 ColorBegin, ColorEnd;
            float Rotation = 0.0f;
            float SizeBegin, SizeEnd;

            float LifeTime = 1.0f;
            float LifeRemaining = 0.0f;

            bool Active = false;

            Ref<Texture2D> Texture = nullptr;
        };

        std::vector<Particle> m_ParticlePool;
        uint32_t m_PoolIndex = 999;
    };
}