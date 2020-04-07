#include "hzpch.h"
#include "Framebuffer.h"

#include "Renderer.h"

#include "Platform/OpenGL/OpenGLFramebuffer.h"

namespace Hazel {

	Hazel::Framebuffer* Framebuffer::Create(FramebufferType type, uint32_t width, uint32_t height)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!")
					return nullptr;

			case RendererAPI::API::OpenGL:
				return new OpenGLFramebuffer(type, width, height);

			default:
				HZ_CORE_ASSERT(false, "Unknown RendererAPI!")
					return nullptr;
		}
	}

}