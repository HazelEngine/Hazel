#pragma once

namespace Hazel {

	enum class FramebufferType { Texture2D, Renderbuffer };

	class Texture2D;

	class HAZEL_API Framebuffer
	{
	public:
		virtual ~Framebuffer() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual void BlitTo(const Framebuffer const* framebuffer) const = 0;

		virtual const std::shared_ptr<Texture2D>& GetBuffer(int index) const = 0;

		static Framebuffer* Create(FramebufferType type, uint32_t width, uint32_t height);
	};

}