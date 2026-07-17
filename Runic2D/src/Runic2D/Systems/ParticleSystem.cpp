#include "R2Dpch.h"
#include "ParticleSystem.h"

#include "Runic2D/Scene/Entity.h"
#include "Runic2D/Scene/Scene.h"
#include "Runic2D/Scene/Components/CoreComponents.h"

#include "Runic2D/Core/Threading/JobSystem.h"
#include "Runic2D/Utils/Random.h"

#include "Runic2D/Renderer/Renderer2D.h" 

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp> 

namespace Runic2D {

	ParticleSystem::ParticleSystem(uint32_t maxParticles)
		: m_PoolIndex(maxParticles - 1)
	{
		m_ParticlePool.resize(maxParticles);
	}

	void ParticleSystem::OnUpdate(Timestep ts, Scene* scene)
	{
		if (scene->IsPaused()) return;

		R2D_PROFILE_SCOPE("Particle System: OnUpdate");

		std::vector<Particle*> activeParticles;
		activeParticles.reserve(m_ParticlePool.size());

		for (auto& particle : m_ParticlePool)
		{
			if (particle.Active)
				activeParticles.emplace_back(&particle);
		}

		if (activeParticles.empty()) return;

		uint32_t count = (uint32_t)activeParticles.size();
		uint32_t groupSize = 128;

		auto stats = JobSystem::Dispatch(count, groupSize, [&activeParticles, ts](uint32_t start, uint32_t end)
			{
				R2D_PROFILE_SCOPE("Particles Job");

				for (uint32_t i = start; i < end; i++)
				{
					Particle& particle = *activeParticles[i];

					particle.LifeRemaining -= ts;

					if (particle.LifeRemaining <= 0.0f)
					{
						particle.Active = false;
						continue;
					}

					particle.Position.x += particle.Velocity.x * ts;
					particle.Position.y += particle.Velocity.y * ts;

					particle.Rotation += 0.01f * ts;
				}
			});

		if (stats.GroupsDispatched > 0)
			JobSystem::Wait();
	}

	void ParticleSystem::OnRender(Scene* scene)
	{
		R2D_PROFILE_SCOPE("Particle System: OnRender");

		auto cameraEntity = scene->GetPrimaryCameraEntity();
		if (cameraEntity) {

			auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
			auto& camTransform = cameraEntity.GetComponent<TransformComponent>();

			Renderer2D::BeginScene(camera, camTransform.GetTransform());

			for (auto& particle : m_ParticlePool)
			{
				if (!particle.Active)
					continue;

				float life = particle.LifeRemaining / particle.LifeTime;

				glm::vec4 color = glm::lerp(particle.ColorEnd, particle.ColorBegin, life);

				float size = glm::lerp(particle.SizeEnd, particle.SizeBegin, life);

				if (particle.Texture)
					Renderer2D::DrawRotatedQuad(particle.Position, { size, size }, particle.Rotation, particle.Texture, 1.0f, color);
				else
					Renderer2D::DrawRotatedQuad(particle.Position, { size, size }, particle.Rotation, color);
			}

			Renderer2D::EndScene();
		}	
	}

	void ParticleSystem::Emit(const ParticleProps& particleProps)
	{
		Particle& particle = m_ParticlePool[m_PoolIndex];

		particle.Active = true;
		particle.Position = particleProps.Position;
		particle.Rotation = Random::Float() * 2.0f * glm::pi<float>();

		particle.Velocity = particleProps.Velocity;
		particle.Velocity.x += particleProps.VelocityVariation.x * (Random::Float() - 0.5f);
		particle.Velocity.y += particleProps.VelocityVariation.y * (Random::Float() - 0.5f);

		particle.ColorBegin = particleProps.ColorBegin;
		particle.ColorEnd = particleProps.ColorEnd;

		particle.LifeTime = particleProps.LifeTime;
		particle.LifeRemaining = particleProps.LifeTime;

		particle.SizeBegin = particleProps.SizeBegin + particleProps.SizeVariation * (Random::Float() - 0.5f);
		particle.SizeEnd = particleProps.SizeEnd;

		particle.Texture = particleProps.Texture;

		m_PoolIndex = --m_PoolIndex % m_ParticlePool.size();
	}

	int ParticleSystem::GetActiveParticleCount() const
	{
		int count = 0;
		for (const auto& particle : m_ParticlePool)
		{
			if (particle.Active)
				count++;
		}
		return count;
	}

}
