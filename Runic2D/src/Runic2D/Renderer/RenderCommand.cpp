#include "R2Dpch.h"
#include "RenderCommand.h"

#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace Runic2D {

	RendererAPI* RenderCommand::s_RendererAPI = new OpenGLRendererAPI;

} // namespace Runic2D