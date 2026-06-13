#pragma once
#include "Runic2D/Scene/ScriptableEntity.h"

#include "Systems/MovementSystem.h"
#include "Systems/EnemySystem.h"
#include "Systems/KnockbackSystem.h"
#include "Systems/DamageFlashSystem.h"

using namespace Runic2D;

namespace Survivor {

    // Aquest script és el 'Cap' del Nivell
    class SurvivorLevelManager : public ScriptableEntity
    {
    public:
        virtual void OnCreate() override
        {
            auto scene = GetScene();

            scene->AddSystem(std::make_shared<DamageFlashSystem>(), { SystemPhase::Logic });
            scene->AddSystem(std::make_shared<KnockbackSystem>(), { SystemPhase::Logic });
            scene->AddSystem(std::make_shared<EnemySystem>(), { SystemPhase::Logic });
            scene->AddSystem(std::make_shared<MovementSystem>(), { SystemPhase::Logic });
        }
    };
}