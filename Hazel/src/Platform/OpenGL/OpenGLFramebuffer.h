#pragma once

#include "Hazel/Renderer/Framebuffer.h"

namespace Hazel {

	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(FramebufferType type, uint32_t width, uint32_t height);
		virtual ~OpenGLFramebuffer();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void BlitTo(const Framebuffer* const framebuffer) const override;

		inline virtual const std::shared_ptr<Texture2D>& GetBuffer(int index) const override
		{
			return m_Buffers[index];
		}

	private:
		uint32_t m_RendererId;
		FramebufferType m_Type;
		uint32_t m_Width, m_Height;
		std::vector<std::shared_ptr<Texture2D>> m_Buffers;
	};

}