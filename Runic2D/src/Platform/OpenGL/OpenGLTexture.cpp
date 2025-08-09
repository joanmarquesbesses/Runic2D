#include "R2Dpch.h"
#include "OpenGLTexture.h"

#include "stb_image.h"

#include <glad/glad.h>

namespace Runic2D {

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
		: m_Path(path)
	{
		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);

		// Load image data
		int width, height, channels;
		stbi_set_flip_vertically_on_load(true); // Flip the image vertically to match OpenGL's texture coordinate system
		unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		if (data)
		{
			m_Width = width;
			m_Height = height;

			glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
			glTextureStorage2D(m_RendererID, 1, GL_RGBA8, width, height);

			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTextureSubImage2D(m_RendererID, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);
		}
		else
		{
			R2D_CORE_ERROR("Failed to load texture: {0}", path);
			stbi_image_free(data);
		}

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, m_RendererID);
	}

	void OpenGLTexture2D::Unbind() const
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}