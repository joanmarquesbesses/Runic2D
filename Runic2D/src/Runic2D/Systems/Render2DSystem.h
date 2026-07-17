#pragma once
#include "System.h"

#include "glm/glm.hpp"

namespace Runic2D {
	class RUNIC_API Render2DSystem : public System
	{
	public:
		virtual ~Render2DSystem() override {};
		virtual void OnRender(Scene* scene) override;

		void SetCustomCamera(const glm::mat4& vp) {
			m_CustomViewProj = vp;
			m_UseCustomCamera = true;
		}

		void ClearCustomCamera() {
			m_UseCustomCamera = false;
		}

	private:
		glm::mat4 m_CustomViewProj = glm::mat4(1.0f);
		bool m_UseCustomCamera = false;
	};
}


