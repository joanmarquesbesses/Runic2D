#pragma once
#include <unordered_map>
#include <vector>
#include <entt.hpp>
#include <glm/glm.hpp>

namespace Runic2D {

	class SpatialHash {
	public:
		SpatialHash(float cellSize) : m_CellSize(cellSize) {}

		void Clear() {
			for (auto& pair : m_Grid) {
				pair.second.clear();
			}
		}

		void Insert(entt::entity entity, glm::vec2 position) {
			uint64_t key = GetKey(position);
			m_Grid[key].push_back(entity);
		}

		void GetNearby(glm::vec2 position, float radius, std::vector<entt::entity>& outEntities) const {
			outEntities.clear();
			int cellX = (int)std::floor(position.x / m_CellSize);
			int cellY = (int)std::floor(position.y / m_CellSize);

			int range = (int)std::ceil(radius / m_CellSize);

			for (int x = -range; x <= range; x++) {
				for (int y = -range; y <= range; y++) {
					uint64_t key = GetKey(cellX + x, cellY + y);
					auto it = m_Grid.find(key);
					if (it != m_Grid.end()) {
						outEntities.insert(outEntities.end(), it->second.begin(), it->second.end());
					}
				}
			}
		}

		const auto& GetGrid() const { return m_Grid; }
		float GetCellSize() const { return m_CellSize; }

		void GetCellCoordinates(uint64_t key, int& outX, int& outY) const {
			outX = (int)(key >> 32);
			outY = (int)(key & 0xFFFFFFFF);
		}

	private:
		uint64_t GetKey(int x, int y) const {
			return ((uint64_t)(uint32_t)x << 32) | (uint32_t)y;
		}

		uint64_t GetKey(glm::vec2 position) const {
			return GetKey((int)std::floor(position.x / m_CellSize), (int)std::floor(position.y / m_CellSize));
		}

		float m_CellSize;
		std::unordered_map<uint64_t, std::vector<entt::entity>> m_Grid;
	};

}