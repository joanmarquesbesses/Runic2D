#pragma once

#include "Runic2D/Core/Base/Core.h"
#include "NavGrid.h"
#include <glm/glm.hpp>
#include <vector>

namespace Runic2D {

    class RUNIC_API AStar {
    public:
        static std::vector<glm::vec2> FindPath(const NavGrid& grid, const glm::vec2& startPos, const glm::vec2& targetPos);
    };

}
