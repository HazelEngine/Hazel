#pragma once

#include "Hazel/Renderer/Texture.h"

#include <glad/glad.h>

namespace Hazel {

	class OpenGLTexture2D : public Texture2D
	{
	public:
		OpenGLTexture2D(const std::string& path);
		OpenGLTexture2D(const void* data, uint32_t width, uint32_t height, uint32_t channels);
		virtual ~OpenGLTexture2D();

		virtual void Bind(uint32_t slot = 0) const override;

		virtual void SetData(void* data, uint32_t size) override;

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		
		uint32_t GetId() const { return m_RendererId; }

	public:
		bool operator==(const Texture& other) const override
		{
			return m_RendererId == ((OpenGLTexture2D&)other).m_RendererId;
		}
		
	private:
		void Create(const void* data, uint32_t width, uint32_t height, uint32_t channels);

	private:
		std::string m_Path;
		uint32_t m_RendererId;
		uint32_t m_Width, m_Height;
		GLenum m_InternalFormat, m_DataFormat;
	};

}