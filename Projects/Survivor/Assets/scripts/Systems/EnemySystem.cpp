#include "SurvivorPCH.h"
#include "EnemySystem.h"

#include "Runic2D/Scene/Scene.h"

#include "Core/GameComponents.h"
#include "Runic2D/Scene/Components/CoreComponents.h"

using namespace Runic2D;

namespace Survivor {
	void EnemySystem::OnUpdate(Timestep ts, Scene* scene)
	{
        Entity playerEntity = scene->GetEntityWithComponent<PlayerStatsComponent>();
        if (!playerEntity) return;

        glm::vec2 playerPos = playerEntity.GetComponent<TransformComponent>().GetTranslation();

        auto view = scene->GetEntityRegistry().view<EnemyStatsComponent, MovementComponent, TransformComponent>(entt::exclude<KnockbackComponent, DeadTag>);
        view.each([&](auto entity, auto& stats, auto& mov, auto& transform)
            {
                glm::vec2 myPos = transform.GetTranslation();
                glm::vec2 difference = playerPos - myPos;
                float distance = glm::length(difference);
                if (distance > 0.1f)
                {
                    mov.direction = glm::normalize(difference);
                    mov.speed = stats.Speed;
                    if (mov.direction.x < 0) {
                        transform.SetScale({ fabs(transform.GetScale().x), transform.GetScale().y, transform.GetScale().z });
                    }
                    else {
                        transform.SetScale({ -fabs(transform.GetScale().x), transform.GetScale().y, transform.GetScale().z });
                    }
                }
                else
                {
                    mov.direction = { 0.0f, 0.0f }; 
                }
            });

        /*per fer proves de profiling, aquest es amb threads
        Entity playerEntity = scene->GetEntityWithComponent<PlayerStatsComponent>();
        if (!playerEntity) return;

        glm::vec2 playerPos = playerEntity.GetComponent<TransformComponent>().GetTranslation();

        struct EnemyEntry
        {
            EnemyStatsComponent* Stats;
            MovementComponent* Mov;
            TransformComponent* Transform;
        };

        auto view = scene->GetEntityRegistry().view<EnemyStatsComponent, MovementComponent, TransformComponent>();

        std::vector<EnemyEntry> entries;
        entries.reserve(view.size_hint());

        view.each([&](auto entity, auto& stats, auto& mov, auto& transform)
            {
                entries.emplace_back(&stats, &mov, &transform);
            });

        if (entries.empty()) return;

        uint32_t count = (uint32_t)entries.size();
        uint32_t groupSize = 64;

        auto stats = JobSystem::Dispatch(count, groupSize, [&entries, playerPos](uint32_t start, uint32_t end)
            {
                for (uint32_t i = start; i < end; i++)
                {
                    auto& entry = entries[i];

                    glm::vec2 myPos = entry.Transform->GetTranslation();
                    glm::vec2 difference = playerPos - myPos;
                    float distance = glm::length(difference);
                    if (distance > 0.1f)
                    {
                        entry.Mov->direction = glm::normalize(difference);
                        entry.Mov->speed = entry.Stats->Speed;
                        if (entry.Mov->direction.x < 0) {
                            entry.Transform->SetScale({ fabs(entry.Transform->GetScale().x), entry.Transform->GetScale().y, entry.Transform->GetScale().z });
                        }
                        else {
                            entry.Transform->SetScale({ -fabs(entry.Transform->GetScale().x), entry.Transform->GetScale().y, entry.Transform->GetScale().z });
                        }
                    }
                    else
                    {
                        entry.Mov->direction = { 0.0f, 0.0f };
                    }
                }
            }
        );

        if (stats.GroupsDispatched > 0)
        {
            Runic2D::JobSystem::Wait();
        }
        */
	}
}

