#pragma once

#include "Hazel/Renderer/Texture.h"

namespace Hazel {

	class HAZEL_API OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::string& path);
		virtual ~OpenGLTexture2D();

		virtual void Bind(uint32_t slot) const override;

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }

	private:
		std::string m_Path;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererId;
	};

}