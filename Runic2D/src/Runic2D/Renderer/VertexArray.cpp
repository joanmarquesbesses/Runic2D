#include "R2Dpch.h"
#include "VertexArray.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace Runic2D {

	VertexArray* Runic2D::VertexArray::Create()
	{

		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:    R2D_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
		case RendererAPI::OpenGL:  return new OpenGLVertexArray();
		}

		R2D_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;

	};

} // namespace Runic2D
