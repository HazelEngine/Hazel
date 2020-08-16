#pragma once

#include "Hazel/Renderer/Texture.h"

#include <glad/glad.h>

namespace Hazel {

	class OpenGLBaseTexture
	{
	public:
		virtual const std::string& GetSamplerName() const = 0;
		virtual void SetSamplerName(const std::string& sampler) = 0;
	};

	class OpenGLTexture2D : public Texture2D, public OpenGLBaseTexture
	{
	public:
		OpenGLTexture2D(
			TextureFormat format,
			uint32_t width,
			uint32_t height,
			void* data,
			TextureWrap wrap = TextureWrap::Clamp
		);

		OpenGLTexture2D(const std::string& path, bool srgb = false);

		virtual ~OpenGLTexture2D();

		virtual void Bind(uint32_t slot = 0) const override;

		virtual void SetData(void* data, uint32_t size) override;

		virtual void Lock() override;
		virtual void Unlock() override;
		virtual void Resize(uint32_t width, uint32_t height) override;
		
		virtual Buffer GetWriteableBuffer() override;

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }
		
		virtual bool Loaded() const override { return m_Loaded; };
		
		virtual TextureFormat GetFormat() const override { return m_Format; };
		// This function currently returns the expected number of mips based on image size,
		// not present mips in data
		virtual uint32_t GetMipLevelCount() const override;

		virtual const std::string& GetPath() const override { return m_FilePath; };

		uint32_t GetRendererId() const override { return m_RendererId; }

		const std::string& GetSamplerName() const override { return m_SamplerName; }
		void SetSamplerName(const std::string& sampler) override { m_SamplerName = sampler; }

	public:
		bool operator==(const Texture& other) const override
		{
			return m_RendererId == ((OpenGLTexture2D&)other).m_RendererId;
		}
		
	private:
		void Create(const void* data, bool srgb);

	private:
		std::string m_FilePath;

		uint32_t m_RendererId;
		uint32_t m_Width, m_Height;
		
		TextureFormat m_Format;
		TextureWrap m_Wrap = TextureWrap::Clamp;

		bool m_IsHDR = false;

		bool m_Locked = false;
		bool m_Loaded = false;

		Buffer m_ImageData;

		std::string m_SamplerName;
	};

	class OpenGLTextureCube : public TextureCube, public OpenGLBaseTexture
	{
	public:
		OpenGLTextureCube(TextureFormat format, uint32_t width, uint32_t height);
		OpenGLTextureCube(const std::string& path);
		virtual ~OpenGLTextureCube();

		virtual void Bind(uint32_t slot = 0) const;

		virtual void SetData(void* data, uint32_t size) override;

		virtual TextureFormat GetFormat() const { return m_Format; }
		virtual uint32_t GetWidth() const { return m_Width; }
		virtual uint32_t GetHeight() const { return m_Height; }
		// This function currently returns the expected number of mips based on image size,
		// not present mips in data
		virtual uint32_t GetMipLevelCount() const override;

		virtual const std::string& GetPath() const override { return m_FilePath; }

		uint32_t GetRendererId() const override { return m_RendererId; }

		const std::string& GetSamplerName() const override { return m_SamplerName; }
		void SetSamplerName(const std::string& sampler) override { m_SamplerName = sampler; }

	public:
		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererId == ((OpenGLTextureCube&)other).m_RendererId;
		}

	private:
		void Create(std::array<unsigned char*, 6> faces);

	private:
		std::string m_FilePath;

		uint32_t m_RendererId;
		TextureFormat m_Format;
		uint32_t m_Width, m_Height;
		uint32_t m_FaceWidth, m_FaceHeight;
		
		unsigned char* m_ImageData;

		std::string m_SamplerName;
	};
}