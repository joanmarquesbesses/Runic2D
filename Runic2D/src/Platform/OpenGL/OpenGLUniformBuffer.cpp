#include "R2Dpch.h"
#include "OpenGLUniformBuffer.h"

#include <glad/glad.h>

namespace Runic2D {

	OpenGLUniformBuffer::OpenGLUniformBuffer(uint32_t size, uint32_t binding)
	{
		// Creem el buffer amb l'estil modern d'OpenGL 4.5 (DSA)
		glCreateBuffers(1, &m_RendererID);

		// Reservem la memòria a la GPU (DYNAMIC_STORAGE perquè l'actualitzarem cada frame)
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW);

		// El vinculem al punt d'unió (Binding Point) especificat (ex: 0 per la Càmera)
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
	}

	OpenGLUniformBuffer::~OpenGLUniformBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void OpenGLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		// Pugem les dades al buffer existent
		glNamedBufferSubData(m_RendererID, offset, size, data);
	}
}