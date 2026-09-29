#include "R2Dpch.h"
#include "SubTexture2D.h"


namespace Runic2D
{
	SubTexture2D::SubTexture2D(const Ref<Texture2D>& texture, const glm::vec2& min, const glm::vec2& max)
		: m_Texture(texture)
	{
		m_TexCoords[0] = { min.x, min.y };
		m_TexCoords[1] = { max.x, min.y };
		m_TexCoords[2] = { max.x, max.y };
		m_TexCoords[3] = { min.x, max.y };
	}

    Ref<SubTexture2D> SubTexture2D::CreateFromPixelCoords(const Ref<Texture2D>& texture, float x, float y, float width, float height)
    {
        float texWidth = (float)texture->GetWidth();
        float texHeight = (float)texture->GetHeight();

        float y_GL = texHeight - y - height;

        glm::vec2 min = {
            x / texWidth,
            y_GL / texHeight
        };

        glm::vec2 max = {
            (x + width) / texWidth,
            (y_GL + height) / texHeight
        };

        return CreateRef<SubTexture2D>(texture, min, max);
    }

    Ref<SubTexture2D> SubTexture2D::CreateFromPixelCoords(const Ref<Texture2D>& texture, const glm::vec2& coords, const glm::vec2& cellSize,
        const glm::vec2& spriteSize)
    {
        float bleedOffset = 0.5f;
        glm::vec2 min = {
            ((coords.x * cellSize.x) + bleedOffset) / texture->GetWidth(),
            ((coords.y * cellSize.y) + bleedOffset) / texture->GetHeight()
        };
        glm::vec2 max = {
            (((coords.x + spriteSize.x) * cellSize.x) - bleedOffset) / texture->GetWidth(),
            (((coords.y + spriteSize.y) * cellSize.y) - bleedOffset) / texture->GetHeight()
        };
        return CreateRef<SubTexture2D>(texture, min, max);
    }
}

