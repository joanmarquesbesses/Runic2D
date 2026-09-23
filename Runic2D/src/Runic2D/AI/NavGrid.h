#pragma once

#include "Runic2D/Core/Base/Core.h"
#include <glm/glm.hpp>
#include <vector>

namespace Runic2D {

    class Scene;

    class RUNIC_API NavGrid {
    public:
        NavGrid() = default;
        NavGrid(int width, int height, float cellSize, glm::vec2 origin);

        void Init(int width, int height, float cellSize, glm::vec2 origin);

        void SetWalkable(int x, int y, bool walkable);
        bool IsWalkable(int x, int y) const;

        // Converteix coordenades del Món a Coordenades de la graella (Columna, Fila)
        bool GetGridCoordinates(const glm::vec2& worldPos, int& outX, int& outY) const;
        
        // Converteix coordenades de graella al centre de la cel·la al món
        glm::vec2 GetWorldPosition(int x, int y) const;

        int GetWidth() const { return m_Width; }
        int GetHeight() const { return m_Height; }
        float GetCellSize() const { return m_CellSize; }
        glm::vec2 GetOrigin() const { return m_Origin; }

        // Escaneja l'escena i marca com a 'false' aquelles cel·les que intersectin amb un Collider
        void GenerateFromScene(Scene* scene);

    private:
        int m_Width = 0;
        int m_Height = 0;
        float m_CellSize = 1.0f;
        glm::vec2 m_Origin = { 0.0f, 0.0f };
        
        // 1D array per cache-friendly data. Accés: y * m_Width + x
        std::vector<bool> m_Walkable;
    };

}
