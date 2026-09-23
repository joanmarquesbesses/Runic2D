#include "R2Dpch.h"
#include "AStar.h"
#include <queue>
#include <unordered_map>
#include <cmath>
#include <algorithm>

namespace Runic2D {

    struct Node {
        int x, y;
        float gCost, hCost;
        Node* parent;

        Node(int x, int y, float gCost, float hCost, Node* parent)
            : x(x), y(y), gCost(gCost), hCost(hCost), parent(parent) {}

        float fCost() const { return gCost + hCost; }
    };

    struct CompareNode {
        bool operator()(Node* a, Node* b) {
            return a->fCost() > b->fCost(); // Menor cost F primer
        }
    };

    static float GetDistance(int x1, int y1, int x2, int y2) {
        int dx = std::abs(x1 - x2);
        int dy = std::abs(y1 - y2);
        // Distància Octil (permet diagonals amb cost 1.414, rectes amb cost 1)
        if (dx > dy)
            return 1.414f * dy + 1.0f * (dx - dy);
        return 1.414f * dx + 1.0f * (dy - dx);
    }

    std::vector<glm::vec2> AStar::FindPath(const NavGrid& grid, const glm::vec2& startPos, const glm::vec2& targetPos)
    {
        int startX, startY, targetX, targetY;
        
        if (!grid.GetGridCoordinates(startPos, startX, startY)) return {};
        if (!grid.GetGridCoordinates(targetPos, targetX, targetY)) return {};

        if (!grid.IsWalkable(targetX, targetY)) return {}; // Destí impossible

        std::priority_queue<Node*, std::vector<Node*>, CompareNode> openSet;
        
        // Utilitzem un flat array per velocitat en lloc d'un unordered_map
        int totalCells = grid.GetWidth() * grid.GetHeight();
        std::vector<bool> closedSet(totalCells, false);
        std::vector<float> gCosts(totalCells, std::numeric_limits<float>::infinity());
        
        std::vector<Node*> allNodes; // Per netejar la memòria al final
        allNodes.reserve(1000); 

        Node* startNode = new Node(startX, startY, 0.0f, GetDistance(startX, startY, targetX, targetY), nullptr);
        openSet.push(startNode);
        allNodes.push_back(startNode);

        int startIndex = startY * grid.GetWidth() + startX;
        gCosts[startIndex] = 0.0f;

        Node* targetNode = nullptr;

        // Direccions (8-way)
        int dirX[] = { 0, 1, 0, -1, 1, 1, -1, -1 };
        int dirY[] = { 1, 0, -1, 0, 1, -1, 1, -1 };

        while (!openSet.empty())
        {
            Node* current = openSet.top();
            openSet.pop();

            if (current->x == targetX && current->y == targetY)
            {
                targetNode = current;
                break;
            }

            int currentIndex = current->y * grid.GetWidth() + current->x;
            closedSet[currentIndex] = true;

            for (int i = 0; i < 8; i++)
            {
                int neighborX = current->x + dirX[i];
                int neighborY = current->y + dirY[i];

                if (!grid.IsWalkable(neighborX, neighborY))
                    continue;

                // Evitem tallar cantonades
                if (i >= 4) // És un moviment diagonal
                {
                    if (!grid.IsWalkable(current->x, neighborY) || !grid.IsWalkable(neighborX, current->y))
                        continue; // Hi ha una paret al costat de la diagonal
                }

                int neighborIndex = neighborY * grid.GetWidth() + neighborX;
                if (closedSet[neighborIndex])
                    continue;

                float moveCost = (i >= 4) ? 1.414f : 1.0f;
                float newGCost = current->gCost + moveCost;

                if (newGCost < gCosts[neighborIndex])
                {
                    gCosts[neighborIndex] = newGCost;
                    float hCost = GetDistance(neighborX, neighborY, targetX, targetY);
                    
                    Node* neighborNode = new Node(neighborX, neighborY, newGCost, hCost, current);
                    openSet.push(neighborNode);
                    allNodes.push_back(neighborNode);
                }
            }
        }

        std::vector<glm::vec2> path;
        if (targetNode != nullptr)
        {
            Node* curr = targetNode;
            while (curr != nullptr)
            {
                path.push_back(grid.GetWorldPosition(curr->x, curr->y));
                curr = curr->parent;
            }
            std::reverse(path.begin(), path.end());
        }

        for (Node* n : allNodes)
            delete n;

        return path;
    }

}
