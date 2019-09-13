#include "hzpch.h"
#include "OpenGLFramebuffer.h"

#include "Hazel/Renderer/Texture.h"

#include "OpenGLTexture.h"

#include <glad/glad.h>
#include <memory>

namespace Hazel {

	OpenGLFramebuffer::OpenGLFramebuffer(FramebufferType type, uint32_t width, uint32_t height)
		: m_RendererId(0), m_Type(type), m_Width(width), m_Height(height)
	{
		glGenFramebuffers(1, &m_RendererId);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId);

		switch (type)
		{
			case FramebufferType::Texture2D:
			{
				auto tex = Texture2D::Create(nullptr, width, height);
				auto ogltex = std::dynamic_pointer_cast<OpenGLTexture2D>(tex);

				if (ogltex == nullptr)
				{
					HZ_CORE_ASSERT(true, "Texture2D is not compatible!")
					return;
				}

				glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ogltex->GetId(), 0);
				m_Buffers.push_back(tex);
			} break;

			case FramebufferType::Renderbuffer:
			{
				uint32_t rbo;
				glGenRenderbuffers(1, &rbo);
				glBindRenderbuffer(GL_RENDERBUFFER, rbo);
				glRenderbufferStorageMultisample(GL_RENDERBUFFER, 8, GL_RGB, width, height);
				glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
			} break;

			default:
				HZ_CORE_ASSERT(false, "Unknown Framebuffer type!")
		}

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			HZ_CORE_ASSERT(false, "Failed to create Framebuffer!")
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	OpenGLFramebuffer::~OpenGLFramebuffer()
	{
		glDeleteFramebuffers(1, &m_RendererId);
	}

	void OpenGLFramebuffer::Bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId);
	}

	void OpenGLFramebuffer::Unbind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void OpenGLFramebuffer::BlitTo(const Framebuffer const* framebuffer) const
	{
		auto glFb = static_cast<const OpenGLFramebuffer*>(framebuffer);

		if (glFb == nullptr)
		{
			HZ_CORE_ASSERT(true, "Framebuffer is not compatible!")
			return;
		}

		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_RendererId);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, glFb->m_RendererId);
		glBlitFramebuffer(
			0, 0, m_Width, m_Height,
			0, 0, glFb->m_Width, glFb->m_Height,
			GL_COLOR_BUFFER_BIT, GL_NEAREST
		);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

}