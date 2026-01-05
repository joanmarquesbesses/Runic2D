#include "R2Dpch.h"
#include "Animation2D.h"

namespace Runic2D {

	Animation2D::Animation2D(const std::vector<Ref<SubTexture2D>>& frames, float frameTime)
		: m_Frames(frames), m_FrameTime(frameTime)
	{
	}

	Ref<SubTexture2D> Animation2D::GetFrame(uint32_t index) const
	{
		if (index >= m_Frames.size()) return nullptr;
		return m_Frames[index];
	}

	Ref<Animation2D> Animation2D::CreateFromAtlas(Ref<Texture2D> atlas, const glm::vec2& cellSize, const glm::vec2& startCoords, int count, float frameTime)
	{
		std::vector<Ref<SubTexture2D>> frames;
		for (int i = 0; i < count; i++)
		{
			frames.push_back(SubTexture2D::CreateFromPixelCoords(atlas, { startCoords.x + i * cellSize.x, startCoords.y }, cellSize));
		}
		return CreateRef<Animation2D>(frames, frameTime);
	}
}