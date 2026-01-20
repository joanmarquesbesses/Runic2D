#pragma once

#include "Runic2D/Core/Timestep.h"
#include "Texture.h"

#include <vector>
#include <glm/glm.hpp>

namespace Runic2D {

    struct ParticleProps
    {
        glm::vec2 Position;
        glm::vec2 Velocity, VelocityVariation;
        glm::vec4 ColorBegin, ColorEnd;
        float SizeBegin, SizeEnd, SizeVariation;
        float LifeTime = 1.0f;
		Ref<Texture2D> Texture = nullptr;
    };

    class ParticleSystem
    {
    public:
        ParticleSystem(uint32_t maxParticles = 10000);

        void OnUpdate(Timestep ts);
        void OnRender();

        void Emit(const ParticleProps& particleProps);

    private:
        struct Particle
        {
            glm::vec2 Position;
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