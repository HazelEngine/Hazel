#include "hzpch.h"
#include "VulkanRenderPass.h"

#include <Hazel/Renderer/Renderer.h>

namespace Hazel {

	VulkanRenderPass::VulkanRenderPass(const RenderPassSpecification& spec)
		: m_Specification(spec) {}

}