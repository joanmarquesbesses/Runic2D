#pragma once

#include <glm/glm.hpp>

#include "VertexArray.h"

namespace Runic2D {

	enum class BlendMode
	{
		Alpha,
		Additive,
		Multiply
	};

	class RUNIC_API RendererAPI
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
		virtual ~RendererAPI() = default;
		virtual void Init() = 0;
		virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
		virtual void SetClearColor(const glm::vec4& color) = 0;
		virtual void Clear() = 0;
	
		virtual void DrawIndexed(const Ref<class VertexArray>& vertexArray, uint32_t indexCount = 0) = 0;
		virtual void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) = 0;

		virtual void SetLineWidth(float width) = 0;

		virtual void DisableScissor() = 0;
		virtual void ClearDepth() = 0;

		virtual void SetBlendMode(BlendMode mode) = 0;
		virtual int GetBoundFramebuffer() = 0;
		virtual void BindFramebuffer(uint32_t framebufferID) = 0;

		virtual void EnableEntityIDWriting(bool enable) = 0;

		virtual void SetDepthMask(bool mask) = 0;

		virtual void ClearStencil() = 0;
		virtual void BeginStencilWrite() = 0;
		virtual void BeginStencilTest() = 0;
		virtual void DisableStencil() = 0;

		inline static API GetAPI() { return s_API; }
	private:
		static API s_API;
	};

} // namespace Runic2D
