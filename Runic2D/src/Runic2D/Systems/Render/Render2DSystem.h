#pragma once
#include "Runic2D/Systems/System.h"

#include "glm/glm.hpp"

#include "Runic2D/Renderer/FrameBuffer.h"
#include "Runic2D/Renderer/Texture.h"

#include "Runic2D/Renderer/Camera.h"

namespace Runic2D {
	class RUNIC_API Render2DSystem : public System
	{
	public:
		Render2DSystem();
		virtual ~Render2DSystem() override {};
		virtual void OnRender(Scene* scene) override;

		void SetCustomCamera(const glm::mat4& vp) {
			m_CustomViewProj = vp;
			m_UseCustomCamera = true;
		}

		void ClearCustomCamera() {
			m_UseCustomCamera = false;
		}

		void ResizeLightmap(uint32_t width, uint32_t height);

		int GetShadowBlurIterations() const { return m_ShadowBlurIterations; }
		int GetBloomIterations() const { return m_BloomIterations; }
		void SetBloomIterations(int iterations) { m_BloomIterations = iterations; }
		float GetBloomIntensity() const { return m_BloomIntensity; }
		void SetBloomIntensity(float intensity) { m_BloomIntensity = intensity; }
		float GetBloomThreshold() const { return m_BloomThreshold; }
		void SetBloomThreshold(float threshold) { m_BloomThreshold = threshold; }
		void SetShadowBlurIterations(int iterations) { m_ShadowBlurIterations = iterations; }

		bool IsLightingEnabled() const { return m_LightingEnabled; }
		void SetLightingEnabled(bool enabled) { m_LightingEnabled = enabled; }
		bool IsShadowsEnabled() const { return m_ShadowsEnabled; }
		void SetShadowsEnabled(bool enabled) { m_ShadowsEnabled = enabled; }
		bool IsBloomEnabled() const { return m_BloomEnabled; }
		void SetBloomEnabled(bool enabled) { m_BloomEnabled = enabled; }

	private:
		struct RenderCamera
		{
			bool Valid = false;
			bool IsCustom = false;
			glm::mat4 ViewProj = glm::mat4(1.0f);
			Camera* CameraPtr = nullptr;
			glm::mat4 Transform = glm::mat4(1.0f);
			void Begin() const;
		};

		RenderCamera GetActiveCamera(Scene* scene);

		void RenderSceneGeometry(Scene* scene, const RenderCamera& camera);
		void RenderTilemaps(Scene* scene);
		void RenderLightmap(Scene* scene, const RenderCamera& camera);
		void RenderShadowsForLight(Scene* scene, const glm::vec3& lightPos, float lightRadius, const RenderCamera& camera);
		void ApplyLightmapToScene();
		void RenderText(Scene* scene, const RenderCamera& camera);
		void RenderPostProcessing(int destinationFBO);

	private:
		glm::mat4 m_CustomViewProj = glm::mat4(1.0f);
		bool m_UseCustomCamera = false;

		uint32_t m_ViewportWidth = 1280, m_ViewportHeight = 720;

		Ref<FrameBuffer> m_LightmapFBO;
		Ref<FrameBuffer> m_MainHDR_FBO;

		Ref<Texture2D> m_LightmapTexture;
		Ref<Texture2D> m_MainHDRTexture;

		int m_ShadowBlurIterations = 4;
		int m_BloomIterations = 5;
		float m_BloomIntensity = 2.5f;
		float m_BloomThreshold = 1.0f;

		bool m_LightingEnabled = true;
		bool m_ShadowsEnabled = true;
		bool m_BloomEnabled = true;
	};
}


