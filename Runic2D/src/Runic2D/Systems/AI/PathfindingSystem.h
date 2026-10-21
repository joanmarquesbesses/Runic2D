#pragma once

#include "Runic2D/Systems/System.h"
#include "Runic2D/AI/NavGrid.h"

namespace Runic2D {

    class RUNIC_API PathfindingSystem : public System
    {
    public:
        virtual void OnStart(Scene* scene) override;
        virtual void OnUpdate(Timestep ts, Scene* scene) override;
        
        Ref<NavGrid> GetNavGrid() const { return m_NavGrid; }

        float GetFlowFieldUpdateInterval() const { return m_FlowFieldUpdateInterval; }
        void SetFlowFieldUpdateInterval(float interval) { m_FlowFieldUpdateInterval = interval; }

    private:
        Ref<NavGrid> m_NavGrid;

        // Optimització de recalcul del Flow Field (Doble filtre: Temps + Posició)
        float m_FlowFieldTimer = 0.0f;
        float m_FlowFieldUpdateInterval = 0.1f;
        int m_LastPlayerCellX = -9999;
        int m_LastPlayerCellY = -9999;
    };

}
