#pragma once

#include "Runic2D/Systems/System.h"

#include "glm/glm.hpp"

namespace Runic2D {

    struct DebugStats
    {
        uint32_t TotalEntities = 0;
        uint32_t ScriptUpdates = 0;
        uint32_t ActiveParticles = 0;
    };

    class RUNIC_API DebugSystem : public System
    {
    private:
        bool m_ShowColliders = false;
        bool m_ShowStats = false;
        glm::mat4 m_CustomViewProj = glm::mat4(1.0f);
        bool m_UseCustomCamera = false;
        bool s_ShowFlockingGrid = false;
        bool m_ShowCustomShadowCaster = false;
        bool m_ShowNavGrid = false;

    public:
        virtual void OnRender(Scene* scene) override;

        void DrawCameraBounds(Scene* scene);

        void SetShowColliders(bool show) { m_ShowColliders = show; }
        bool GetShowColliders() const { return m_ShowColliders; }

        void SetShowStats(bool show) { m_ShowStats = show; }
        bool GetShowStats() const { return m_ShowStats; }

        void SetShowFlockingGrid(bool show) { s_ShowFlockingGrid = show; }
        bool GetShowFlockingGrid() const { return s_ShowFlockingGrid; }

        void SetShowCustomShadowCaster(bool show) { m_ShowCustomShadowCaster = show; }
        bool GetShowCustomShadowCaster() const { return m_ShowCustomShadowCaster; }

        void SetShowNavGrid(bool show) { m_ShowNavGrid = show; }
        bool GetShowNavGrid() const { return m_ShowNavGrid; }

        void SetCustomCamera(const glm::mat4& vp) {
            m_CustomViewProj = vp;
            m_UseCustomCamera = true;
        }

        void ClearCustomCamera() {
            m_UseCustomCamera = false;
        }

        DebugStats GetStats(Scene* scene) const;
    };
}
