#pragma once

#include <Hazel/Renderer/RenderPass.h>

namespace Hazel {

	class HAZEL_API OpenGLRenderPass : public RenderPass
	{
	public:
		OpenGLRenderPass(const RenderPassSpecification& spec);
		virtual ~OpenGLRenderPass() = default;

		virtual const RenderPassSpecification& GetSpecification() const { return m_Specification; }

	private:
		RenderPassSpecification m_Specification;
	};

}