#include "hzpch.h"
#include "OpenGLTexture.h"

#include <glad/glad.h>
#include "stb_image.h"

namespace Hazel {

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path)
		: m_Path(path), m_Width(0), m_Height(0)
	{
		stbi_set_flip_vertically_on_load(true);

		int width, height, channels;
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
		HZ_CORE_ASSERT(data, "Failed to load image!")

		Create(data, width, height, channels);

		stbi_image_free(data);
	}

	OpenGLTexture2D::OpenGLTexture2D(
		const void* data,
		uint32_t width,
		uint32_t height,
		uint32_t channels
	) : m_Path(""), m_Width(width), m_Height(height)
	{
		Create(data, width, height, channels);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		glDeleteTextures(1, &m_RendererId);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererId);
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		HZ_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture!")
		glTextureSubImage2D(m_RendererId, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::Create(const void* data, uint32_t width, uint32_t height, uint32_t channels)
	{
		m_Width = width;
		m_Height = height;

		if (channels == 3)
		{
			m_InternalFormat = GL_RGB8;
			m_DataFormat = GL_RGB;
		}
		else if (channels == 4)
		{
			m_InternalFormat = GL_RGBA8;
			m_DataFormat = GL_RGBA;
		}

		HZ_CORE_ASSERT(m_InternalFormat & m_DataFormat, "Format not supported!")

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererId);
		glTextureStorage2D(m_RendererId, 1, m_InternalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_RendererId, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
	}

}