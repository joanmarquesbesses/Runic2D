#include "R2Dpch.h"
#include "Shader.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace Runic2D {

	Shader* Shader::Create(const std::string& vertexSource, const std::string& fragmentSource)
	{

		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:    R2D_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
			case RendererAPI::API::OpenGL:  return new OpenGLShader(vertexSource, fragmentSource);
		}

		R2D_CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;

	};

} // namespace Runic2D