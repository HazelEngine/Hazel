#include "hzpch.h"
#include "RenderPass.h"

#include "Renderer.h"

#include "Platform/OpenGL/OpenGLRenderPass.h"
#include "Platform/Vulkan/VulkanRenderPass.h"

namespace Hazel {

	Ref<RenderPass> RenderPass::Create(const RenderPassSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!")
					return nullptr;

			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLRenderPass>(spec);

			case RendererAPI::API::Vulkan:
				return CreateRef<VulkanRenderPass>(spec);

			default:
				HZ_CORE_ASSERT(false, "Unknown RendererAPI!")
					return nullptr;
		}
	}
	
}
