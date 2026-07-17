#include "R2Dpch.h"
#include "AudioSystem.h"

#include "Runic2D/Scene/Scene.h"
#include "Runic2D/Scene/Components/AudioComponents.h"
#include "Runic2D/Audio/AudioEngine.h"

namespace Runic2D {

	void AudioSystem::OnStart(Scene* scene)
	{
		auto& registry = scene->GetEntityRegistry();
		auto view = registry.view<AudioSourceComponent>();

		for (auto entity : view)
		{
			auto& asc = view.get<AudioSourceComponent>(entity);
			if (asc.Clip)
			{
				asc.RuntimeHandle = AudioEngine::CreateSource(asc.Clip, AudioGroup::SFX, asc.Looping);
				if (asc.RuntimeHandle != 0)
				{
					AudioEngine::SetSourceVolume(asc.RuntimeHandle, asc.Volume);
					AudioEngine::SetSourcePitch(asc.RuntimeHandle, asc.Pitch);
					
					if (asc.PlayOnAwake)
					{
						AudioEngine::PlaySource(asc.RuntimeHandle);
					}
				}
			}
		}
	}

	void AudioSystem::OnStop(Scene* scene)
	{
		auto& registry = scene->GetEntityRegistry();
		auto view = registry.view<AudioSourceComponent>();

		for (auto entity : view)
		{
			auto& asc = view.get<AudioSourceComponent>(entity);
			if (asc.RuntimeHandle != 0)
			{
				AudioEngine::DestroySource(asc.RuntimeHandle);
				asc.RuntimeHandle = 0;
			}
		}
	}

	void AudioSystem::OnUpdate(Timestep ts, Scene* scene)
	{
		auto& registry = scene->GetEntityRegistry();
		auto view = registry.view<AudioSourceComponent>();

		for (auto entity : view)
		{
			auto& asc = view.get<AudioSourceComponent>(entity);
			if (asc.RuntimeHandle != 0)
			{
				// Actualitzem propietats que poden haver canviat per codi/UI
				AudioEngine::SetSourceVolume(asc.RuntimeHandle, asc.Volume);
				AudioEngine::SetSourcePitch(asc.RuntimeHandle, asc.Pitch);

				// Processar requests
				if (asc.RequestPlay) {
					AudioEngine::PlaySource(asc.RuntimeHandle);
					asc.RequestPlay = false;
				}
				if (asc.RequestStop) {
					AudioEngine::StopSource(asc.RuntimeHandle);
					asc.RequestStop = false;
				}
				if (asc.RequestPause) {
					AudioEngine::PauseSource(asc.RuntimeHandle);
					asc.RequestPause = false;
				}
			}
		}
	}

}
