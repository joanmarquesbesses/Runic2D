#pragma once

#include "Runic2D/Renderer/RendererAPI.h"

namespace Runic2D
{
	class OpenGLRendererAPI : public RendererAPI 
	{
	public:
		virtual void Init() override;

		virtual void SetClearColor(const glm::vec4& color) override;

		virtual void Clear() override;

		virtual void DrawIndexed(const Ref<class VertexArray>& vertexArray) override;
	};
}