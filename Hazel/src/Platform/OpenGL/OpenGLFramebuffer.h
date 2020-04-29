#pragma once

#include "Hazel/Renderer/Framebuffer.h"

namespace Hazel {

	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferSpecification& spec);
		virtual ~OpenGLFramebuffer() = default;

		virtual const FramebufferSpecification& GetSpecification() const { return m_Specification; }

	private:
		FramebufferSpecification m_Specification;
	};

}