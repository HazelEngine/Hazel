#pragma once

#include "Hazel/Renderer/GraphicsContext.h"

struct GLFWwindow;

namespace Hazel {

	class HAZEL_API OpenGLContext : public GraphicsContext
	{
	public:
		OpenGLContext(GLFWwindow* window);

		virtual void Init() override;
		virtual void SwapBuffers() override;

	private:
		GLFWwindow* m_Window;
	};

}