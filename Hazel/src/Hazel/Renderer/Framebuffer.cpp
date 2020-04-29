#include "hzpch.h"
#include "Framebuffer.h"

#include "Renderer.h"

#include "Platform/OpenGL/OpenGLFramebuffer.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"

namespace Hazel {
	
	Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!")
					return nullptr;

			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLFramebuffer>(spec);

			case RendererAPI::API::Vulkan:
				return CreateRef<VulkanFramebuffer>(spec);

			default:
				HZ_CORE_ASSERT(false, "Unknown RendererAPI!")
					return nullptr;
		}
	}
	
}
