#include "R2Dpch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace Runic2D
{

	OpenGLContext::OpenGLContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		R2D_CORE_ASSERT(windowHandle, "Window handle is null!");
	}

	void OpenGLContext::Init()
	{
		R2D_PROFILE_FUNCTION();

		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		R2D_CORE_ASSERT(status, "Failed to initialize Glad!");

		R2D_CORE_INFO("  Vendor {0}", (const char*)glGetString(GL_VERSION));
		R2D_CORE_INFO("  Renderer {0}", (const char*)glGetString(GL_RENDERER));
		R2D_CORE_INFO("  GLSL Version {0}", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
	}

	void OpenGLContext::SwapBuffers()
	{
		R2D_PROFILE_FUNCTION();

		glfwSwapBuffers(m_WindowHandle);
	}

}