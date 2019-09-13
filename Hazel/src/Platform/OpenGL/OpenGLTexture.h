#pragma once

#include "Hazel/Renderer/Texture.h"

namespace Hazel {

	class HAZEL_API OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::string& path);
		OpenGLTexture2D(const void* data, uint32_t width, uint32_t height, uint32_t channels);
		virtual ~OpenGLTexture2D();

		virtual void Bind(uint32_t slot) const override;

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		
		uint32_t GetId() const { return m_RendererId; }

	private:
		void Create(const void* data, uint32_t width, uint32_t height, uint32_t channels);

	private:
		std::string m_Path;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererId;
	};

}