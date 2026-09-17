#pragma once

#include "RendererAPI.h"

namespace Runic2D {

	class RUNIC_API RenderCommand
	{
	public:
		inline static void Init()
		{
			s_RendererAPI->Init();
		}

		inline static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
		{
			s_RendererAPI->SetViewport(x, y, width, height);
		}

		inline static void SetClearColor(const glm::vec4& color)
		{
			s_RendererAPI->SetClearColor(color);
		}

		inline static void Clear()
		{
			s_RendererAPI->Clear();
		}

		inline static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0)
		{
			s_RendererAPI->DrawIndexed(vertexArray, indexCount);
		}

		static void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
		{
			s_RendererAPI->DrawLines(vertexArray, vertexCount);
		}

		static void SetLineWidth(float width)
		{
			s_RendererAPI->SetLineWidth(width);
		}

		inline static void DisableScissor()
		{
			s_RendererAPI->DisableScissor();
		}

		inline static void ClearDepth()
		{
			s_RendererAPI->ClearDepth();
		}

		inline static void SetBlendMode(BlendMode mode)
		{
			s_RendererAPI->SetBlendMode(mode);
		}

		inline static int GetBoundFramebuffer() 
		{ 
			return s_RendererAPI->GetBoundFramebuffer(); 
		}

		inline static void BindFramebuffer(uint32_t framebufferID) 
		{ 
			s_RendererAPI->BindFramebuffer(framebufferID); 
		}

		inline static void EnableEntityIDWriting(bool enable) 
		{ 
			s_RendererAPI->EnableEntityIDWriting(enable); 
		}

		static void SetDepthMask(bool mask) 
		{ 
			s_RendererAPI->SetDepthMask(mask);
		}

	private:
		static RendererAPI* s_RendererAPI;
	};

} // namespace Runic2D
