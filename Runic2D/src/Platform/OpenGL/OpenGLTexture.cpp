#include "R2Dpch.h"
#include "OpenGLTexture.h"

#include "stb_image.h"
#include "Runic2D/Core/Threading/BackgroundTaskSystem.h"
#include "Runic2D/Core/App/Application.h"

#include <glad/glad.h>

namespace Runic2D {

	namespace Utils {

		static GLenum RunicImageFormatToGLDataFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RGB8:  return GL_RGB;
			case ImageFormat::RGBA8: return GL_RGBA;
			}

			R2D_CORE_ASSERT(false);
			return 0;
		}

		static GLenum RunicImageFormatToGLInternalFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RGB8:  return GL_RGB8;
			case ImageFormat::RGBA8: return GL_RGBA8;
			}

			R2D_CORE_ASSERT(false);
			return 0;
		}

	}

	OpenGLTexture2D::OpenGLTexture2D(const TextureSpecification& specification)
		: m_Specification(specification), m_Width(m_Specification.Width), m_Height(m_Specification.Height)
	{
		R2D_PROFILE_FUNCTION();

		m_InternalFormat = Utils::RunicImageFormatToGLInternalFormat(m_Specification.Format);
		m_DataFormat = Utils::RunicImageFormatToGLDataFormat(m_Specification.Format);

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
		: m_Path(path) , m_Width(1), m_Height(1)
	{
		R2D_PROFILE_FUNCTION();

		int w = 1, h = 1, channels = 4;
		if (stbi_info(path.c_str(), &w, &h, &channels))
		{
			m_Width = w;
			m_Height = h;
		}

		// 1. Create a 1x1 white texture synchronously on the main thread (Placeholder)
		m_InternalFormat = GL_RGBA8;
		m_DataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, 1, 1);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		uint32_t whiteTextureData = 0xffffffff;
		glTextureSubImage2D(m_RendererID, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &whiteTextureData);

		// 2. Dispatch a job to load the actual image from disk without blocking
		BackgroundTaskSystem::Execute([this, path]() {
			int width, height, channels;
			stbi_set_flip_vertically_on_load(true);
			stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);

			if (data) {
				// 3. Submit back to main thread to upload the pixels to OpenGL
				Application::Get().SubmitToMainThread([this, data, width, height, channels]() {
					m_Width = width;
					m_Height = height;

					if (channels == 4) {
						m_InternalFormat = GL_RGBA8;
						m_DataFormat = GL_RGBA;
					} else if (channels == 3) {
						m_InternalFormat = GL_RGB8;
						m_DataFormat = GL_RGB;
					}

					// We have to recreate the texture storage because we can't resize immutable storage
					glDeleteTextures(1, &m_RendererID);

					glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
					glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

					glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
					glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

					glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);

					stbi_image_free(data);
				});
			} else {
				R2D_CORE_WARN("Failed to async load texture: {0}", path);
			}
		});
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		R2D_PROFILE_FUNCTION();

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

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		R2D_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture!");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}
}
