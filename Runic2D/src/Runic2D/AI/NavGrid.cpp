#include "R2Dpch.h"
#include "NavGrid.h"
#include "Runic2D/Scene/Scene.h"
#include "Runic2D/Scene/Entity.h"
#include "Runic2D/Scene/Components/CoreComponents.h"
#include "Runic2D/Scene/Components/PhysicsComponents.h"

namespace Runic2D {

    NavGrid::NavGrid(int width, int height, float cellSize, glm::vec2 origin) 
    {
        Init(width, height, cellSize, origin);
    }

    void NavGrid::Init(int width, int height, float cellSize, glm::vec2 origin)
    {
        m_Width = width;
        m_Height = height;
        m_CellSize = cellSize;
        m_Origin = origin;
        m_Walkable.assign(m_Width * m_Height, true);
    }

    void NavGrid::SetWalkable(int x, int y, bool walkable)
    {
        if (x >= 0 && x < m_Width && y >= 0 && y < m_Height)
            m_Walkable[y * m_Width + x] = walkable;
    }

    bool NavGrid::IsWalkable(int x, int y) const
    {
        if (x >= 0 && x < m_Width && y >= 0 && y < m_Height)
            return m_Walkable[y * m_Width + x];
        return false;
    }

    bool NavGrid::GetGridCoordinates(const glm::vec2& worldPos, int& outX, int& outY) const
    {
        outX = static_cast<int>(std::floor((worldPos.x - m_Origin.x) / m_CellSize));
        outY = static_cast<int>(std::floor((worldPos.y - m_Origin.y) / m_CellSize));
        return (outX >= 0 && outX < m_Width && outY >= 0 && outY < m_Height);
    }

    glm::vec2 NavGrid::GetWorldPosition(int x, int y) const
    {
        return glm::vec2(
            m_Origin.x + (x * m_CellSize) + (m_CellSize * 0.5f),
            m_Origin.y + (y * m_CellSize) + (m_CellSize * 0.5f)
        );
    }

    void NavGrid::GenerateFromScene(Scene* scene)
    {
        if (!scene) return;
        
        // Netejem el grid (posem tot a true)
        m_Walkable.assign(m_Width * m_Height, true);

        auto view = scene->GetEntityRegistry().view<TransformComponent, BoxCollider2DComponent>();
        view.each([&](auto entity, auto& transform, auto& bc2d)
        {
            if (bc2d.IsSensor) return;

            // Filtrem només els elements estàtics si tenen Rigidbody
            if (scene->GetEntityRegistry().all_of<Rigidbody2DComponent>(entity))
            {
                auto& rb2d = scene->GetEntityRegistry().get<Rigidbody2DComponent>(entity);
                if (rb2d.Type != Rigidbody2DComponent::BodyType::Static)
                    return; // Només les parets estàtiques bloquegen el pathfinding generalment
            }

            // Per respectar Jerarquies, Rotacions i Escales del pare, creem la matriu exacta:
            Entity e{ entity, scene };
            glm::mat4 worldTransform = e.GetWorldTransform();

            glm::mat4 colliderTransform = worldTransform
                * glm::translate(glm::mat4(1.0f), glm::vec3(bc2d.Offset, 0.0f))
                * glm::scale(glm::mat4(1.0f), glm::vec3(bc2d.Size, 1.0f));

            // Les 4 cantonades d'un quadrat unitari (1x1) centrat
            glm::vec4 corners[4] = {
                { -0.5f, -0.5f, 0.0f, 1.0f },
                {  0.5f, -0.5f, 0.0f, 1.0f },
                {  0.5f,  0.5f, 0.0f, 1.0f },
                { -0.5f,  0.5f, 0.0f, 1.0f }
            };

            float minX = std::numeric_limits<float>::max();
            float maxX = std::numeric_limits<float>::lowest();
            float minY = std::numeric_limits<float>::max();
            float maxY = std::numeric_limits<float>::lowest();

            // Transformem les cantonades a espai del món i trobem l'AABB extrem
            for (int i = 0; i < 4; i++) {
                glm::vec3 worldPos = colliderTransform * corners[i];
                if (worldPos.x < minX) minX = worldPos.x;
                if (worldPos.x > maxX) maxX = worldPos.x;
                if (worldPos.y < minY) minY = worldPos.y;
                if (worldPos.y > maxY) maxY = worldPos.y;
            }

            // Busquem a quines cel·les del grid cau aquest AABB real al món
            int startGridX = std::max(0, static_cast<int>(std::floor((minX - m_Origin.x) / m_CellSize)));
            int startGridY = std::max(0, static_cast<int>(std::floor((minY - m_Origin.y) / m_CellSize)));
            int endGridX = std::min(m_Width - 1, static_cast<int>(std::floor((maxX - m_Origin.x) / m_CellSize)));
            int endGridY = std::min(m_Height - 1, static_cast<int>(std::floor((maxY - m_Origin.y) / m_CellSize)));

            for (int y = startGridY; y <= endGridY; ++y)
            {
                for (int x = startGridX; x <= endGridX; ++x)
                {
                    SetWalkable(x, y, false); // Marquem la cel·la com a mur
                }
            }
        });

        auto viewCircle = scene->GetEntityRegistry().view<TransformComponent, CircleCollider2DComponent>();
        viewCircle.each([&](auto entity, auto& transform, auto& cc2d)
        {
            if (cc2d.IsSensor) return;

            if (scene->GetEntityRegistry().all_of<Rigidbody2DComponent>(entity))
            {
                auto& rb2d = scene->GetEntityRegistry().get<Rigidbody2DComponent>(entity);
                if (rb2d.Type != Rigidbody2DComponent::BodyType::Static)
                    return;
            }

            Entity e{ entity, scene };
            glm::mat4 worldTransform = e.GetWorldTransform();

            // Calculem l'escala global usant la longitud dels vectors columna de la matriu
            glm::vec3 worldScale = {
                glm::length(glm::vec3(worldTransform[0])),
                glm::length(glm::vec3(worldTransform[1])),
                glm::length(glm::vec3(worldTransform[2]))
            };
            float maxScale = std::max(std::abs(worldScale.x), std::abs(worldScale.y));
            float worldRadius = cc2d.Radius * maxScale;

            // Offset aplica sobre l'escala abans de rotar/translacionar
            glm::vec3 worldCenter = worldTransform * glm::vec4(cc2d.Offset, 0.0f, 1.0f);

            float minX = worldCenter.x - worldRadius;
            float maxX = worldCenter.x + worldRadius;
            float minY = worldCenter.y - worldRadius;
            float maxY = worldCenter.y + worldRadius;

            float tolerance = 0.0f; // Un valor petit (ajustable al teu gust)
            int startGridX = std::max(0, static_cast<int>(std::floor((minX + tolerance - m_Origin.x) / m_CellSize)));
            int startGridY = std::max(0, static_cast<int>(std::floor((minY + tolerance - m_Origin.y) / m_CellSize)));
            int endGridX = std::min(m_Width - 1, static_cast<int>(std::floor((maxX - tolerance - m_Origin.x) / m_CellSize)));
            int endGridY = std::min(m_Height - 1, static_cast<int>(std::floor((maxY - tolerance - m_Origin.y) / m_CellSize)));

            for (int y = startGridY; y <= endGridY; ++y)
            {
                for (int x = startGridX; x <= endGridX; ++x)
                {
                    SetWalkable(x, y, false);
                }
            }
        });

        auto viewPolygon = scene->GetEntityRegistry().view<TransformComponent, PolygonCollider2DComponent>();
        viewPolygon.each([&](auto entity, auto& transform, auto& pc2d)
        {
            if (pc2d.IsSensor || pc2d.Vertices.size() < 3) return;

            if (scene->GetEntityRegistry().all_of<Rigidbody2DComponent>(entity))
            {
                auto& rb2d = scene->GetEntityRegistry().get<Rigidbody2DComponent>(entity);
                if (rb2d.Type != Rigidbody2DComponent::BodyType::Static)
                    return;
            }

            Entity e{ entity, scene };
            glm::mat4 worldTransform = e.GetWorldTransform();

            float minX = std::numeric_limits<float>::max();
            float maxX = std::numeric_limits<float>::lowest();
            float minY = std::numeric_limits<float>::max();
            float maxY = std::numeric_limits<float>::lowest();

            std::vector<glm::vec2> worldVertices(pc2d.Vertices.size());

            for (size_t i = 0; i < pc2d.Vertices.size(); i++)
            {
                glm::vec3 worldPos = worldTransform * glm::vec4(pc2d.Vertices[i] + pc2d.Offset, 0.0f, 1.0f);
                worldVertices[i] = { worldPos.x, worldPos.y };

                if (worldPos.x < minX) minX = worldPos.x;
                if (worldPos.x > maxX) maxX = worldPos.x;
                if (worldPos.y < minY) minY = worldPos.y;
                if (worldPos.y > maxY) maxY = worldPos.y;
            }

            int startGridX = std::max(0, static_cast<int>(std::floor((minX - m_Origin.x) / m_CellSize)));
            int startGridY = std::max(0, static_cast<int>(std::floor((minY - m_Origin.y) / m_CellSize)));
            int endGridX = std::min(m_Width - 1, static_cast<int>(std::floor((maxX - m_Origin.x) / m_CellSize)));
            int endGridY = std::min(m_Height - 1, static_cast<int>(std::floor((maxY - m_Origin.y) / m_CellSize)));

            // Per a cada casella dins de l'AABB, mirem si realment cau DINS del polígon
            for (int y = startGridY; y <= endGridY; ++y)
            {
                for (int x = startGridX; x <= endGridX; ++x)
                {
                    glm::vec2 cellCenter = m_Origin + glm::vec2(x * m_CellSize + (m_CellSize * 0.5f), y * m_CellSize + (m_CellSize * 0.5f));
                    float h = m_CellSize * 0.5f;

                    auto isPointInPoly = [&](const glm::vec2& p) {
                        bool inside = false;
                        for (size_t i = 0, j = worldVertices.size() - 1; i < worldVertices.size(); j = i++)
                        {
                            if (((worldVertices[i].y > p.y) != (worldVertices[j].y > p.y)) &&
                                (p.x < (worldVertices[j].x - worldVertices[i].x) * (p.y - worldVertices[i].y) / (worldVertices[j].y - worldVertices[i].y) + worldVertices[i].x))
                            {
                                inside = !inside;
                            }
                        }
                        return inside;
                    };

                    bool blocked = false;

                    // 1. Comprovem el centre i les 4 cantonades de la casella
                    if (isPointInPoly(cellCenter) ||
                        isPointInPoly(cellCenter + glm::vec2(-h, -h)) ||
                        isPointInPoly(cellCenter + glm::vec2(h, -h)) ||
                        isPointInPoly(cellCenter + glm::vec2(-h, h)) ||
                        isPointInPoly(cellCenter + glm::vec2(h, h)))
                    {
                        blocked = true;
                    }

                    // 2. Comprovem si algun vèrtex del polígon cau a DINS de la casella
                    if (!blocked) {
                        for (const auto& v : worldVertices) {
                            if (v.x >= cellCenter.x - h && v.x <= cellCenter.x + h && 
                                v.y >= cellCenter.y - h && v.y <= cellCenter.y + h) 
                            {
                                blocked = true;
                                break;
                            }
                        }
                    }

                    if (blocked) {
                        SetWalkable(x, y, false);
                    }
                }
            }
        });
    }
}
