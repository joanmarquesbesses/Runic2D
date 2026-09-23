#include "R2Dpch.h"
#include "PathfindingSystem.h"

#include "Runic2D/Scene/Scene.h"
#include "Runic2D/Scene/Components/CoreComponents.h"
#include "Runic2D/Scene/Components/MotionComponents.h"
#include "Runic2D/Scene/Components/AIComponents.h"

#include "Runic2D/AI/AStar.h"
#include "Runic2D/Core/Threading/JobSystem.h"

namespace Runic2D {

    void PathfindingSystem::OnStart(Scene* scene)
    {
        m_NavGrid = CreateRef<NavGrid>(200, 200, 0.5f, glm::vec2(-50.0f, -50.0f));
        m_NavGrid->GenerateFromScene(scene);
    }

    void PathfindingSystem::OnUpdate(Timestep ts, Scene* scene)
    {
        auto& registry = scene->GetEntityRegistry();
        auto view = registry.view<TransformComponent, MovementComponent, PathfindingComponent>();
        std::vector<entt::entity> entitiesToRepath;
        entitiesToRepath.reserve(view.size_hint());

        view.each([&](auto entity, auto& tc, auto& mc, auto& path)
            {
                if (!path.Enabled || path.TargetEntity == 0) return;
                path.RepathTimer -= ts;
                if (path.RepathTimer <= 0.0f)
                {
                    entitiesToRepath.push_back(entity);
                }
            });

        if (!entitiesToRepath.empty())
        {
            uint32_t count = (uint32_t)entitiesToRepath.size();
            uint32_t groupSize = 4; 
            auto stats = JobSystem::Dispatch(count, groupSize, [&](uint32_t start, uint32_t end)
                {
                    for (uint32_t i = start; i < end; i++)
                    {
                        entt::entity entity = entitiesToRepath[i];
                        auto& path = registry.get<PathfindingComponent>(entity);
                        auto& tc = registry.get<TransformComponent>(entity);
                        Entity target = scene->GetEntityByUUID(path.TargetEntity);
                        if (target)
                        {
                            glm::vec2 targetPos = target.GetComponent<TransformComponent>().GetTranslation();
                            glm::vec2 myPos = tc.GetTranslation();

                            // A* concurrent! Totalment thread-safe.
                            path.Path = AStar::FindPath(*m_NavGrid, myPos, targetPos);
                            path.CurrentWaypointIndex = 0;
                        }

                        float randomOffset = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.1f;
                        path.RepathTimer = path.RepathInterval + randomOffset;
                    }
                });
            if (stats.GroupsDispatched > 0)
                JobSystem::Wait();
        }
        // 3. Fase d'Aplicació del Moviment (Single-thread ràpid, abans que actuï el Flocking)
        view.each([&](auto entity, auto& tc, auto& mc, auto& path)
            {
                if (!path.Enabled || path.TargetEntity == 0) return;
                if (!path.Path.empty() && path.CurrentWaypointIndex < path.Path.size())
                {
                    glm::vec2 myPos = tc.GetTranslation();
                    glm::vec2 waypoint = path.Path[path.CurrentWaypointIndex];

                    float dist = glm::distance(myPos, waypoint);
                    if (dist < 0.2f)
                    {
                        path.CurrentWaypointIndex++;
                    }
                    else
                    {
                        mc.direction = glm::normalize(waypoint - myPos);
                    }
                }
                else
                {
                    mc.direction = { 0.0f, 0.0f };
                }
            });
    }

}
