#include "R2Dpch.h"
#include "FrameBuffer.h"

#include "Runic2D/Renderer/Renderer.h"

#include "Platform/OpenGL/OpenGLFrameBuffer.h"

namespace Runic2D
{
		Ref<FrameBuffer> Runic2D::FrameBuffer::Create(const FrameBufferSpecification& spec)
		{
			switch (Renderer::GetAPI())
			{
				case RendererAPI::API::None:    R2D_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
				case RendererAPI::API::OpenGL:  return CreateRef<OpenGLFrameBuffer>(spec);
			}
		}
}

