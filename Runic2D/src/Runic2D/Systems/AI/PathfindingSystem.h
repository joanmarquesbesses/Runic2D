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

    private:
        Ref<NavGrid> m_NavGrid;
    };

}
