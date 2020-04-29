#pragma once

#include <Hazel/Renderer/Pipeline.h>

#include <vulkan/vulkan.hpp>

namespace Hazel {

	class HAZEL_API VulkanPipeline : public Pipeline
	{
	public:
		VulkanPipeline(const PipelineSpecification& spec);
		virtual ~VulkanPipeline() = default;

		VkPipeline GetVulkanPipeline() const { return m_Pipeline; }

		virtual const PipelineSpecification& GetSpecification() const override { return m_Specification; }

	private:
		PipelineSpecification m_Specification;

		VkPipeline m_Pipeline;
	};

}