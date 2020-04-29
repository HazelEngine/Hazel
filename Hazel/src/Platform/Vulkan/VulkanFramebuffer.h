#pragma once

#include <Hazel/Renderer/Framebuffer.h>

namespace Hazel {

	class HAZEL_API VulkanFramebuffer : public Framebuffer
	{
	public:
		VulkanFramebuffer(const FramebufferSpecification& spec);
		virtual ~VulkanFramebuffer() = default;

		virtual const FramebufferSpecification& GetSpecification() const override { return m_Specification; }

	private:
		FramebufferSpecification m_Specification;
	};

}