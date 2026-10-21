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

        m_IntegrationField.assign(m_Width * m_Height, 0xFFFF);
        m_FlowField.assign(m_Width * m_Height, { 0.0f, 0.0f });
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

    void NavGrid::UpdateFlowField(const glm::vec2& targetWorldPos)
    {
        int targetX, targetY;
        if (!GetGridCoordinates(targetWorldPos, targetX, targetY))
            return;
        int totalCells = m_Width * m_Height;
        // 1. Reset de les distàncies a infinit (0xFFFF)
        std::fill(m_IntegrationField.begin(), m_IntegrationField.end(), (uint16_t)0xFFFF);
        // 2. Si la casella de l'objectiu no és transitable, busquem la més propera
        if (!IsWalkable(targetX, targetY))
        {
            bool found = false;
            for (int r = 1; r <= 3 && !found; r++)
            {
                for (int dx = -r; dx <= r && !found; dx++)
                {
                    for (int dy = -r; dy <= r && !found; dy++)
                    {
                        if (IsWalkable(targetX + dx, targetY + dy))
                        {
                            targetX += dx;
                            targetY += dy;
                            found = true;
                        }
                    }
                }
            }
            if (!found) return;
        }
        // 3. Dijkstra Wavefront (BFS des de l'objectiu cap enfora)
        std::queue<int> openQueue;
        int targetIndex = targetY * m_Width + targetX;
        m_IntegrationField[targetIndex] = 0;
        openQueue.push(targetIndex);
        // 4 direccions rectes cardinals per a la propagació de distàncies
        const int dirX[] = { 0, 1, 0, -1 };
        const int dirY[] = { 1, 0, -1, 0 };
        while (!openQueue.empty())
        {
            int currIndex = openQueue.front();
            openQueue.pop();
            int cx = currIndex % m_Width;
            int cy = currIndex / m_Width;
            uint16_t currDist = m_IntegrationField[currIndex];
            for (int i = 0; i < 4; i++)
            {
                int nx = cx + dirX[i];
                int ny = cy + dirY[i];
                if (nx >= 0 && nx < m_Width && ny >= 0 && ny < m_Height)
                {
                    int neighborIndex = ny * m_Width + nx;
                    if (m_Walkable[neighborIndex] && m_IntegrationField[neighborIndex] == 0xFFFF)
                    {
                        m_IntegrationField[neighborIndex] = currDist + 1;
                        openQueue.push(neighborIndex);
                    }
                }
            }
        }
        // 4. Generació del Vector Field (Cada casella apunta al seu veí amb menor distància)
        // 8 direccions (permet diagonals suaus)
        const int nX[] = { 0, 1, 0, -1, 1, 1, -1, -1 };
        const int nY[] = { 1, 0, -1, 0, 1, -1, 1, -1 };
        for (int y = 0; y < m_Height; y++)
        {
            for (int x = 0; x < m_Width; x++)
            {
                int index = y * m_Width + x;
                if (!m_Walkable[index] || m_IntegrationField[index] == 0xFFFF)
                {
                    m_FlowField[index] = { 0.0f, 0.0f };
                    continue;
                }
                // Si és la casella de l'objectiu, direcció zero
                if (x == targetX && y == targetY)
                {
                    m_FlowField[index] = { 0.0f, 0.0f };
                    continue;
                }
                uint16_t bestDist = m_IntegrationField[index];
                glm::vec2 bestDir = { 0.0f, 0.0f };
                for (int i = 0; i < 8; i++)
                {
                    int nx = x + nX[i];
                    int ny = y + nY[i];
                    if (nx >= 0 && nx < m_Width && ny >= 0 && ny < m_Height)
                    {
                        // Evitem creuar cantonades de parets en diagonals
                        if (i >= 4)
                        {
                            if (!m_Walkable[y * m_Width + nx] || !m_Walkable[ny * m_Width + x])
                                continue;
                        }
                        int nIndex = ny * m_Width + nx;
                        if (m_IntegrationField[nIndex] < bestDist)
                        {
                            bestDist = m_IntegrationField[nIndex];
                            bestDir = glm::normalize(glm::vec2((float)nX[i], (float)nY[i]));
                        }
                    }
                }
                m_FlowField[index] = bestDir;
            }
        }
    }
    glm::vec2 NavGrid::GetFlowDirection(const glm::vec2& worldPos) const
    {
        int x, y;

        if (!GetGridCoordinates(worldPos, x, y))
            return { 0.0f, 0.0f };

        int index = y * m_Width + x;

        // 1. Cas ideal (99.9% de les vegades): la cel·la és caminable i té direcció de flux
        if (m_Walkable[index] && (m_FlowField[index].x != 0.0f || m_FlowField[index].y != 0.0f))
        {
            return m_FlowField[index];
        }

        // 2. Si l'enemic ha quedat empès dins d'un obstacle / paret:
        // Busquem en un petit radi (1 a 3 cel·les) la cel·la transitable més propera
        for (int r = 1; r <= 3; r++)
        {
            for (int dx = -r; dx <= r; dx++)
            {
                for (int dy = -r; dy <= r; dy++)
                {
                    if (std::abs(dx) == r || std::abs(dy) == r)
                    {
                        int nx = x + dx;
                        int ny = y + dy;
                        if (nx >= 0 && nx < m_Width && ny >= 0 && ny < m_Height)
                        {
                            int nIndex = ny * m_Width + nx;
                            if (m_Walkable[nIndex] && (m_FlowField[nIndex].x != 0.0f || m_FlowField[nIndex].y != 0.0f))
                            {
                                return m_FlowField[nIndex]; // Retornem el flux de la cel·la lliure més propera!
                            }
                        }
                    }
                }
            }
        }

        return { 0.0f, 0.0f };
    }
}
