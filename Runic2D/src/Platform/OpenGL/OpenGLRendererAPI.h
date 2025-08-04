#pragma once

#include "Runic2D/Renderer/RendererAPI.h"

namespace Runic2D
{
	class OpenGLRendererAPI : public RendererAPI 
	{
	public:
		virtual void SetClearColor(const glm::vec4& color) override;

		virtual void Clear() override;

		virtual void DrawIndexed(const std::shared_ptr<class VertexArray>& vertexArray) override;
	};
}