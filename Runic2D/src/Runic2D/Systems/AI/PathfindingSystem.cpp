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

        // =========================================================================
        // 1. GESTIÓ DEL FLOW FIELD (Desacoblat: cerca l'objectiu via TargetEntity)
        // =========================================================================
        UUID flowTargetUUID = 0;
        view.each([&](auto entity, auto& tc, auto& mc, auto& path)
            {
                if (flowTargetUUID != 0) return; // Ja l'hem trobat en una volta anterior!

                if (path.Enabled && path.Type == PathfindingType::FlowField && path.TargetEntity != 0)
                {
                    flowTargetUUID = path.TargetEntity;
                }
            }
        );

        if (flowTargetUUID != 0)
        {
            Entity target = scene->GetEntityByUUID(flowTargetUUID);
            if (target)
            {
                m_FlowFieldTimer -= ts;
                if (m_FlowFieldTimer <= 0.0f)
                {
                    glm::vec2 targetPos = target.GetComponent<TransformComponent>().GetTranslation();
                    int targetCellX, targetCellY;

                    if (m_NavGrid->GetGridCoordinates(targetPos, targetCellX, targetCellY))
                    {
                        // Doble filtre: només si canvia de casella a la graella
                        if (targetCellX != m_LastPlayerCellX || targetCellY != m_LastPlayerCellY)
                        {
                            m_NavGrid->UpdateFlowField(targetPos);
                            m_LastPlayerCellX = targetCellX;
                            m_LastPlayerCellY = targetCellY;
                        }
                    }
                    m_FlowFieldTimer = m_FlowFieldUpdateInterval;
                }
            }
        }

        // =========================================================================
        // 2. ASSIGNACIÓ DE DIRECCIONS I RECOLLIDA D'A*
        // =========================================================================

        std::vector<entt::entity> aStarEntitiesToRepath;
        aStarEntitiesToRepath.reserve(view.size_hint()); // Reserva recuperada!

        view.each([&](auto entity, auto& tc, auto& mc, auto& path)
            {
                if (!path.Enabled) return;

                // --- BRANCA A: FLOW FIELD (Ultra-ràpid O(1)) ---
                if (path.Type == PathfindingType::FlowField)
                {
                    glm::vec2 myPos = tc.GetTranslation();
                    mc.direction = m_NavGrid->GetFlowDirection(myPos);
                    return;
                }

                // --- BRANCA B: A* (Rutes individuals / patrulles) ---
                if (path.Type == PathfindingType::AStar)
                {
                    if (path.TargetEntity == 0) return;

                    path.RepathTimer -= ts;
                    if (path.RepathTimer <= 0.0f)
                    {
                        aStarEntitiesToRepath.push_back(entity);
                    }
                }
            }
        );

        // =========================================================================
        // 3. RECALCUL PARAL·LEL D'A* AMB JOBSYSTEM (Només per entitats A*)
        // =========================================================================

        if (!aStarEntitiesToRepath.empty())
        {
            uint32_t count = (uint32_t)aStarEntitiesToRepath.size();
            uint32_t groupSize = 4;
            auto stats = JobSystem::Dispatch(count, groupSize, [&](uint32_t start, uint32_t end)
                {
                    for (uint32_t i = start; i < end; i++)
                    {
                        entt::entity entity = aStarEntitiesToRepath[i];
                        auto& path = registry.get<PathfindingComponent>(entity);
                        auto& tc = registry.get<TransformComponent>(entity);
                        Entity target = scene->GetEntityByUUID(path.TargetEntity);
                        if (target)
                        {
                            glm::vec2 targetPos = target.GetComponent<TransformComponent>().GetTranslation();
                            glm::vec2 myPos = tc.GetTranslation();

                            path.Path = AStar::FindPath(*m_NavGrid, myPos, targetPos);
                            path.CurrentWaypointIndex = (path.Path.size() > 1) ? 1 : 0;
                        }

                        float randomOffset = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 0.1f;
                        path.RepathTimer = path.RepathInterval + randomOffset;
                    }
                }
            );

            if (stats.GroupsDispatched > 0)
                JobSystem::Wait();
        }

        // =========================================================================
        // 4. APLICACIÓ DEL MOVIMENT PER A ENYS EN MODE A* (Waypoints)
        // =========================================================================

        view.each([&](auto entity, auto& tc, auto& mc, auto& path)
            {
                if (!path.Enabled || path.Type != PathfindingType::AStar) return;

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
            }
        );
    }

}