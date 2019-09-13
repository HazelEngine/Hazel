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

	void OpenGLTexture2D::Create(const void* data, uint32_t width, uint32_t height, uint32_t channels)
	{
		m_Width = width;
		m_Height = height;

		GLenum internalFormat = 0, dataFormat = 0;
		if (channels == 3)
		{
			internalFormat = GL_RGB8;
			dataFormat = GL_RGB;
		}
		else if (channels == 4)
		{
			internalFormat = GL_RGBA8;
			dataFormat = GL_RGBA;
		}

		HZ_CORE_ASSERT(internalFormat & dataFormat, "Format not supported!")

			glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererId);
		glTextureStorage2D(m_RendererId, 1, internalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureSubImage2D(m_RendererId, 0, 0, 0, m_Width, m_Height, dataFormat, GL_UNSIGNED_BYTE, data);
	}

}