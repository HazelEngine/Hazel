#pragma once

#include <Hazel/Renderer/RenderPass.h>

#include <vulkan/vulkan.hpp>

namespace Hazel {

	class HAZEL_API VulkanRenderPass : public RenderPass
	{
	public:
		VulkanRenderPass(const RenderPassSpecification& spec);
		virtual ~VulkanRenderPass() = default;

		virtual const RenderPassSpecification& GetSpecification() const override { return m_Specification; }

	private:
		RenderPassSpecification m_Specification;
	};

}