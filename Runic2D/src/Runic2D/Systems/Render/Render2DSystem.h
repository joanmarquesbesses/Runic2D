#pragma once
#include "Runic2D/Systems/System.h"

#include "glm/glm.hpp"

#include "Runic2D/Renderer/FrameBuffer.h"

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

	private:
		glm::mat4 m_CustomViewProj = glm::mat4(1.0f);
		bool m_UseCustomCamera = false;

		Ref<FrameBuffer> m_LightmapFBO;
		uint32_t m_ViewportWidth = 1280, m_ViewportHeight = 720;

		Ref<FrameBuffer> m_MainHDR_FBO;

		int m_ShadowBlurIterations = 4;
		int m_BloomIterations = 10;
		float m_BloomIntensity = 2.5f;
		float m_BloomThreshold = 1.0f;
	};
}


