#include "hzpch.h"
#include "RenderCommandBuffer.h"

#include "Renderer.h"

#include "Platform/OpenGL/OpenGLRenderCommandBuffer.h"
#include "Platform/Vulkan/VulkanRenderCommandBuffer.h"

namespace Hazel {

	Ref<RenderCommandBuffer> RenderCommandBuffer::Create()
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!")
					return nullptr;

			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLRenderCommandBuffer>();

			case RendererAPI::API::Vulkan:
				return CreateRef<VulkanRenderCommandBuffer>();

			default:
				HZ_CORE_ASSERT(false, "Unknown RendererAPI!")
					return nullptr;
		}
	}
	
}
