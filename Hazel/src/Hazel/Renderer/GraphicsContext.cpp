#include "hzpch.h"
#include "GraphicsContext.h"

#include "Renderer.h"
#include "RendererAPI.h"

#include <Platform/OpenGL/OpenGLContext.h>
#include <Platform/Vulkan/VulkanContext.h>

namespace Hazel {

	Scope<GraphicsContext> GraphicsContext::Create(GLFWwindow* window)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!")
				return nullptr;

			case RendererAPI::API::OpenGL:
				return CreateScope<OpenGLContext>(window);

			case RendererAPI::API::Vulkan:
				return CreateScope<VulkanContext>(window);

			default:
				HZ_CORE_ASSERT(false, "Unknown RendererAPI!")
				return nullptr;
		}
	}

}
