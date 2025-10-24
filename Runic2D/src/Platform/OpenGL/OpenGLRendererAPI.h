#pragma once

#include "Runic2D/Renderer/RendererAPI.h"

namespace Runic2D
{
	class OpenGLRendererAPI : public RendererAPI 
	{
	public:
		virtual void Init() override;

		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

		virtual void SetClearColor(const glm::vec4& color) override;

		virtual void Clear() override;

		virtual void DrawIndexed(const Ref<class VertexArray>& vertexArray, uint32_t indexCount = 0) override;
	};
}