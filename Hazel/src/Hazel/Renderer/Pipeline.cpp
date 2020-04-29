#include "hzpch.h"
#include "Pipeline.h"

#include "Renderer.h"

#include "Platform/OpenGL/OpenGLPipeline.h"
#include "Platform/Vulkan/VulkanPipeline.h"

namespace Hazel {
	
	Ref<Pipeline> Pipeline::Create(const PipelineSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
			case RendererAPI::API::None:
				HZ_CORE_ASSERT(false, "RendererAPI::None is currently not supported!")
					return nullptr;

			case RendererAPI::API::OpenGL:
				return CreateRef<OpenGLPipeline>(spec);

			case RendererAPI::API::Vulkan:
				return CreateRef<VulkanPipeline>(spec);

			default:
				HZ_CORE_ASSERT(false, "Unknown RendererAPI!")
					return nullptr;
		}
	}
	
}
