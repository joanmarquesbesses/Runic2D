#pragma once

#include "Runic2D/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Runic2D {

	class OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* windowHandle);

		virtual void Init() override;
		virtual void SwapBuffers() override;

	private:
		GLFWwindow* m_WindowHandle;
	};

} // namespace Runic2D