#include "R2Dpch.h"
#include "OpenGLRendererAPI.h"

#include <glad/glad.h>

namespace Runic2D {

	void OpenGLRendererAPI::Init()
	{
		R2D_PROFILE_FUNCTION();

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LINE_SMOOTH);
	}

	void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void OpenGLRendererAPI::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void OpenGLRendererAPI::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		vertexArray->Bind();
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
	}

	void OpenGLRendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
	{
		vertexArray->Bind();
		glDrawArrays(GL_LINES, 0, vertexCount);
	}

	void OpenGLRendererAPI::SetLineWidth(float width)
	{
		glLineWidth(width);
	}

	void OpenGLRendererAPI::DisableScissor()
	{
		glDisable(GL_SCISSOR_TEST);
	}

	void OpenGLRendererAPI::ClearDepth()
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRendererAPI::SetBlendMode(BlendMode mode)
	{
		switch (mode)
		{
		case BlendMode::Alpha:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		case BlendMode::Additive:
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			break;
		case BlendMode::Multiply:
			glBlendFunc(GL_DST_COLOR, GL_ZERO);
			break;
		}
	}

	int OpenGLRendererAPI::GetBoundFramebuffer()
	{
		int previousFBO = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFBO);
		return previousFBO;
	}

	void OpenGLRendererAPI::BindFramebuffer(uint32_t framebufferID)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
	}

	void OpenGLRendererAPI::EnableEntityIDWriting(bool enable)
	{
		int boundFBO = 0;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &boundFBO);
		if (boundFBO == 0) return;
		if (enable)
		{
			GLenum buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
			glDrawBuffers(2, buffers);
		}
		else
		{
			GLenum buffers[1] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, buffers);
		}
	}

	void OpenGLRendererAPI::SetDepthMask(bool mask)
	{
		glDepthMask(mask ? GL_TRUE : GL_FALSE);
	}

}
