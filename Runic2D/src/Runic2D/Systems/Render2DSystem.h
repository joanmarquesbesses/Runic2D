#pragma once
#include "System.h"

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

	private:
		glm::mat4 m_CustomViewProj = glm::mat4(1.0f);
		bool m_UseCustomCamera = false;

		Ref<FrameBuffer> m_LightmapFBO;
		uint32_t m_ViewportWidth = 1280, m_ViewportHeight = 720;
	};
}


