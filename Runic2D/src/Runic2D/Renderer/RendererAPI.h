#pragma once

#include <glm/glm.hpp>

#include "VertexArray.h"

namespace Runic2D {

	class RendererAPI
	{
	public:
		enum class API
		{
			None = 0,
			OpenGL = 1,
			Vulkan = 2,
			DirectX = 3
		};
	public:
		virtual void Init() = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;
	
		virtual void DrawIndexed(const Ref<class VertexArray>& vertexArray) = 0;

		inline static API GetAPI() { return s_API; }
	private:
		static API s_API;
	};

} // namespace Runic2D