#include "hzpch.h"
#include "OpenGLContext.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <gl/GL.h>

namespace Hazel {

	OpenGLContext::OpenGLContext(GLFWwindow* window)
		: m_Window(window)
	{
		HZ_CORE_ASSERT(window, "Window handle is null!")
	}

	void OpenGLContext::Init()
	{
		glfwMakeContextCurrent(m_Window);
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		HZ_CORE_ASSERT(status, "Failed to initialize GLAD")

		HZ_CORE_INFO("OpenGL Renderer:")
		HZ_CORE_INFO("  Vendor: {0}", glGetString(GL_VENDOR))
		HZ_CORE_INFO("  Renderer: {0}", glGetString(GL_RENDERER))
		HZ_CORE_INFO("  Version: {0}", glGetString(GL_VERSION))
	}

	void OpenGLContext::SwapBuffers()
	{
		glfwSwapBuffers(m_Window);
	}

}