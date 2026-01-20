#pragma once

#include "Runic2D/Renderer/SubTexture2D.h"
#include <vector>

namespace Runic2D {

	class Animation2D
	{
	public:
		Animation2D() = default;
		Animation2D(const std::vector<Ref<SubTexture2D>>& frames, float frameTime);

		Ref<SubTexture2D> GetFrame(uint32_t index) const;
		uint32_t GetFrameCount() const { return (uint32_t)m_Frames.size(); }
		float GetFrameTime() const { return m_FrameTime; }

		static Ref<Animation2D> CreateFromAtlas(Ref<Texture2D> atlas, const glm::vec2& cellSize, const glm::vec2& startCoords, int count, int framesPerRow, float frameTime);

	private:
		std::vector<Ref<SubTexture2D>> m_Frames;
		float m_FrameTime = 0.1f; // Temps per frame (ex: 0.1s = 10fps)
	};
}