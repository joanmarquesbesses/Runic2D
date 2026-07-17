#pragma once

#include "System.h"

namespace Runic2D {

    class RUNIC_API Animation2DSystem : public System
    {
    public:
        virtual ~Animation2DSystem() override {};
        virtual void OnStart(Scene* scene) override;
        virtual void OnUpdate(Timestep ts, Scene* scene) override;
    };
}
