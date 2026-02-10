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

	Ref<Animation2D> Animation2D::CreateFromAtlas(Ref<Texture2D> atlas, const glm::vec2& cellSize, const glm::vec2& startCoords, int count, int framesPerRow, float frameTime)
	{
		std::vector<Ref<SubTexture2D>> frames;

		int validFramesPerRow = framesPerRow;
		if (validFramesPerRow <= 0)
			validFramesPerRow = (int)(atlas->GetWidth() / cellSize.x);

		if (validFramesPerRow < 1) validFramesPerRow = 1;

		int numRowsInTexture = (int)(atlas->GetHeight() / cellSize.y);

		for (int i = 0; i < count; i++)
		{
			int relativeCol = i % validFramesPerRow;
			int relativeRow = i / validFramesPerRow;

			float spriteSheetX = (startCoords.x / cellSize.x) + relativeCol;
			float spriteSheetY = (startCoords.y / cellSize.y) + relativeRow;

			float glRow = (numRowsInTexture - 1) - spriteSheetY;

			glm::vec2 coords = { spriteSheetX, glRow };

			Ref<SubTexture2D> subtexture = SubTexture2D::CreateFromCoords(atlas, coords, cellSize);
			frames.push_back(subtexture);
		}

		return CreateRef<Animation2D>(frames, frameTime);
	}
}