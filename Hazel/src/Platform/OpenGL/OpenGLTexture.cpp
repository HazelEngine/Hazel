#include "hzpch.h"
#include "OpenGLTexture.h"

#include <glad/glad.h>
#include <stb_image.h>

namespace Hazel {

	static GLenum HZToGLTextureFormat(TextureFormat format)
	{
		switch (format)
		{
			case Hazel::TextureFormat::RGB:				return GL_RGB;
			case Hazel::TextureFormat::RGBA:			return GL_RGBA;
			case Hazel::TextureFormat::Float16:			return GL_RGBA16F;
		}

		HZ_CORE_ASSERT(false, "Unknown texture format!")
		return 0;
	}

	static GLenum HZToGLTextureWrap(TextureWrap wrap)
	{
		switch (wrap)
		{
			case Hazel::TextureWrap::Clamp:				return GL_CLAMP_TO_EDGE;
			case Hazel::TextureWrap::Repeat:			return GL_REPEAT;
		}

		HZ_CORE_ASSERT(false, "Unknown texture format!")
		return 0;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// Texture2D
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	OpenGLTexture2D::OpenGLTexture2D(
		TextureFormat format,
		uint32_t width,
		uint32_t height,
		void* data,
		TextureWrap wrap
	) : m_FilePath(""), m_Format(format), m_Width(width), m_Height(height), m_Wrap(wrap)
	{
		HZ_PROFILE_FUNCTION()
		
		Create(data, false);

		if (data == nullptr) {
			uint32_t bpp = Texture::GetBPP(m_Format);
			m_ImageData.Allocate(width * height * bpp);
		}
	}

	OpenGLTexture2D::OpenGLTexture2D(const std::string& path, bool srgb)
		: m_FilePath(path)
	{
		HZ_PROFILE_FUNCTION()

		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		if (stbi_is_hdr(path.c_str()))
		{
			HZ_CORE_INFO("Loading HDR texture {0}, srgb = {1}", path, srgb)
			m_ImageData.Data = (byte*)stbi_loadf(path.c_str(), &width, &height, &channels, 0);
			m_IsHDR = true;
			m_Format = TextureFormat::Float16;
		}
		else
		{
			HZ_CORE_INFO("Loading texture {0}, srgb = {1}", path, srgb)
			int desiredChannels = srgb ? STBI_rgb : STBI_rgb_alpha;
			m_ImageData.Data = stbi_load(path.c_str(), &width, &height, &channels, desiredChannels);
			HZ_CORE_ASSERT(m_ImageData.Data, "Could not read image!")
			m_Format = TextureFormat::RGBA;
		}

		if (!m_ImageData.Data) return;

		m_Loaded = true;
		m_Width = width;
		m_Height = height;

		Create(m_ImageData.Data, srgb);

		stbi_image_free(m_ImageData.Data);
	}

	OpenGLTexture2D::~OpenGLTexture2D()
	{
		HZ_PROFILE_FUNCTION()
		glDeleteTextures(1, &m_RendererId);
	}

	void OpenGLTexture2D::Bind(uint32_t slot) const
	{
		HZ_PROFILE_FUNCTION()
		glBindTextureUnit(slot, m_RendererId);
	}

	void OpenGLTexture2D::SetData(void* data, uint32_t size)
	{
		HZ_PROFILE_FUNCTION()
		uint32_t bpp = Texture::GetBPP(m_Format);
		GLenum format = HZToGLTextureFormat(m_Format);
		HZ_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture!")
		glTextureSubImage2D(m_RendererId, 0, 0, 0, m_Width, m_Height, format, GL_UNSIGNED_BYTE, data);
	}

	void OpenGLTexture2D::Create(const void* data, bool srgb)
	{
		HZ_PROFILE_FUNCTION()

		if (srgb)
		{
			int levels = Texture::CalculateMipMapCount(m_Width, m_Height);
			GLenum minFilter = levels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;

			glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererId);
			glTextureStorage2D(m_RendererId, levels, GL_SRGB8, m_Width, m_Height);
			
			glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, minFilter);
			glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTextureSubImage2D(m_RendererId, 0, 0, 0, m_Width, m_Height, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateTextureMipmap(m_RendererId);
		}
		else
		{
			GLenum internalFormat = HZToGLTextureFormat(m_Format);
			GLenum dataFormat = srgb ? GL_SRGB8 : (m_IsHDR ? GL_RGB : internalFormat); // HDR = GL_RGB for now
			GLenum type = internalFormat == GL_RGBA16F ? GL_FLOAT : GL_UNSIGNED_BYTE;
			GLenum wrap = HZToGLTextureWrap(m_Wrap);

			glGenTextures(1, &m_RendererId);
			glBindTexture(GL_TEXTURE_2D, m_RendererId);

			glTextureParameteri(m_RendererId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTextureParameteri(m_RendererId, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_S, wrap);
			glTextureParameteri(m_RendererId, GL_TEXTURE_WRAP_T, wrap);

			// TODO: Get from capabilities
			// glTextureParameterf(m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, RendererAPI::GetCapabilities().MaxAnisotropy);
			glTextureParameteri(m_RendererId, GL_TEXTURE_MAX_ANISOTROPY, 16);

			glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, dataFormat, type, data);

			glGenerateMipmap(GL_TEXTURE_2D);

			glBindTexture(GL_TEXTURE_2D, 0);
		}

	}

	void OpenGLTexture2D::Lock()
	{
		m_Locked = true;
	}

	void OpenGLTexture2D::Unlock()
	{
		m_Locked = false;
		// TODO: Upload updated buffer data
		// glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, HazelToOpenGLTextureFormat(m_Format), GL_UNSIGNED_BYTE, m_ImageData.Data);
	}

	void OpenGLTexture2D::Resize(uint32_t width, uint32_t height)
	{
		HZ_CORE_ASSERT(m_Locked, "Texture must be locked!")

		uint32_t bpp = Texture::GetBPP(m_Format);
		m_ImageData.Allocate(width * height * bpp);

#if HZ_DEBUG
		m_ImageData.ZeroInitialize();
#endif
	}

	Buffer OpenGLTexture2D::GetWriteableBuffer()
	{
		HZ_CORE_ASSERT(m_Locked, "Texture must be locked!")
		return m_ImageData;
	}

	uint32_t OpenGLTexture2D::GetMipLevelCount() const
	{
		return Texture::CalculateMipMapCount(m_Width, m_Height);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// TextureCube
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	OpenGLTextureCube::OpenGLTextureCube(TextureFormat format, uint32_t width, uint32_t height)
		: m_FilePath(""), m_Format(format), m_Width(width), m_Height(height)
	{
		HZ_PROFILE_FUNCTION()

		m_FaceWidth = m_Width / 4;
		m_FaceHeight = m_Height / 3;

		uint32_t bpp = Texture::GetBPP(m_Format);
		uint32_t faceSize = m_FaceWidth * m_FaceHeight * bpp;
		HZ_CORE_ASSERT(m_FaceWidth == m_FaceHeight, "Non-square faces!")

		std::array<unsigned char*, 6> faces;
		for (size_t i = 0; i < faces.size(); i++) {
			faces[i] = new unsigned char[faceSize];
		}

		int faceIndex = 0;

		// Process +X, +Z, -X, -Z
		for (size_t i = 0; i < 4; i++)
		{
			int colorR = rand() % 256;
			int colorG = rand() % 256;
			int colorB = rand() % 256;

			for (size_t y = 0; y < m_FaceHeight; y++)
			{
				size_t yOffset = y + m_FaceHeight;
				for (size_t x = 0; x < m_FaceWidth; x++)
				{
					size_t index = (x + y * m_FaceWidth) * bpp;
					faces[faceIndex][index + 0] = colorR;
					faces[faceIndex][index + 1] = colorG;
					faces[faceIndex][index + 2] = colorB;
					if (bpp == 4) {	// Alpha
						faces[faceIndex][index + 3] = 0xFF;
					}
				}
			}

			faceIndex++;
		}

		// Process +Y and -Y
		for (size_t i = 0; i < 3; i++)
		{
			// Skip the middle one
			if (i == 1) continue;

			int colorR = rand() % 256;
			int colorG = rand() % 256;
			int colorB = rand() % 256;

			for (size_t y = 0; y < m_FaceHeight; y++)
			{
				for (size_t x = 0; x < m_FaceWidth; x++)
				{
					size_t index = (x + y * m_FaceWidth) * bpp;
					faces[faceIndex][index + 0] = colorR;
					faces[faceIndex][index + 1] = colorG;
					faces[faceIndex][index + 2] = colorB;
					if (bpp == 4) {	// Alpha
						faces[faceIndex][index + 3] = 0xFF;
					}
				}
			}

			faceIndex++;
		}

		// Create the cubemap
		Create(faces);
	}

	OpenGLTextureCube::OpenGLTextureCube(const std::string& path)
		: m_FilePath(path)
	{
		HZ_PROFILE_FUNCTION()

		int width, height, channels;
		stbi_set_flip_vertically_on_load(false);
		m_ImageData = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb);
		HZ_CORE_ASSERT(m_ImageData, "Failed to load image!")

		m_Width = width;
		m_Height = height;
		m_Format = TextureFormat::RGB;

		m_FaceWidth = m_Width / 4;
		m_FaceHeight = m_Height / 3;
		HZ_CORE_ASSERT(m_FaceWidth == m_FaceHeight, "Non-square faces!")

		std::array<unsigned char*, 6> faces;
		for (size_t i = 0; i < faces.size(); i++) {
			faces[i] = new unsigned char[m_FaceWidth * m_FaceHeight * 3]; // 3 BPP
		}

		// Put the face data in sequential order (+X, -X, +Y, -Y, +Z, -Z)
		auto GetFaceIndex = [=](int index) -> int {
			switch (index)
			{
				case 0:		 return 1;
				case 1:		 return 4;
				case 2:		 return 0;
				case 3:		 return 5;
				case 4:		 return 2;
				case 5:		 return 3;
			}
		};

		int processedFaces = 0;

		for (size_t i = 0; i < 4; i++)
		{
			int faceIndex = GetFaceIndex(processedFaces);

			for (size_t y = 0; y < m_FaceHeight; y++)
			{
				size_t yOffset = y + m_FaceHeight;
				for (size_t x = 0; x < m_FaceWidth; x++)
				{
					size_t xOffset = x + i * m_FaceWidth;
					size_t srcCoord = (xOffset + yOffset * m_Width) * 3;
					size_t destCoord = (x + y * m_FaceWidth) * 3;
					faces[faceIndex][destCoord + 0] = m_ImageData[srcCoord + 0];
					faces[faceIndex][destCoord + 1] = m_ImageData[srcCoord + 1];
					faces[faceIndex][destCoord + 2] = m_ImageData[srcCoord + 2];
				}
			}
			processedFaces++;
		}

		for (size_t i = 0; i < 3; i++)
		{
			// Skip the middle one
			if (i == 1) continue;

			int faceIndex = GetFaceIndex(processedFaces);

			for (size_t y = 0; y < m_FaceHeight; y++)
			{
				size_t yOffset = y + i * m_FaceHeight;
				for (size_t x = 0; x < m_FaceWidth; x++)
				{
					size_t xOffset = x + m_FaceWidth;
					size_t srcCoord = (xOffset + yOffset * m_Width) * 3;
					size_t destCoord = (x + y * m_FaceWidth) * 3;
					faces[faceIndex][destCoord + 0] = m_ImageData[srcCoord + 0];
					faces[faceIndex][destCoord + 1] = m_ImageData[srcCoord + 1];
					faces[faceIndex][destCoord + 2] = m_ImageData[srcCoord + 2];
				}
			}
			processedFaces++;
		}

		// Create the cubemap
		Create(faces);

		// And free the raw image data
		stbi_image_free(m_ImageData);
	}

	OpenGLTextureCube::~OpenGLTextureCube()
	{
		HZ_PROFILE_FUNCTION()
		glDeleteTextures(1, &m_RendererId);
	}

	void OpenGLTextureCube::Bind(uint32_t slot) const
	{
		HZ_PROFILE_FUNCTION()
		glBindTextureUnit(slot, m_RendererId);
	}

	void OpenGLTextureCube::SetData(void* data, uint32_t size)
	{
		// TODO: Implement
	}

	uint32_t OpenGLTextureCube::GetMipLevelCount() const
	{
		return Texture::CalculateMipMapCount(m_Width, m_Height);
	}

	void OpenGLTextureCube::Create(std::array<unsigned char*, 6> faces)
	{
		glGenTextures(1, &m_RendererId);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererId);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		// TODO: Get from capabilities
		// glTextureParameterf(m_RendererID, GL_TEXTURE_MAX_ANISOTROPY, RendererAPI::GetCapabilities().MaxAnisotropy);
		glTextureParameteri(m_RendererId, GL_TEXTURE_MAX_ANISOTROPY, 16);

		auto format = HZToGLTextureFormat(m_Format);
		for (size_t i = 0; i < faces.size(); i++)
		{
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				format,
				m_FaceWidth,
				m_FaceHeight,
				0,
				format,
				GL_UNSIGNED_BYTE,
				faces[i]
			);
		}

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

		for (size_t i = 0; i < faces.size(); i++) {
			delete[] faces[i];
		}
	}

}