#pragma once

struct GLFWwindow;

namespace Hazel {

	class HAZEL_API GraphicsContext
	{
	public:
		virtual void Init() = 0;

		virtual void Prepare() = 0;
		virtual void SwapBuffers() = 0;

		static Scope<GraphicsContext> Create(GLFWwindow* window);
	};

}