#pragma once

#include <Hazel/Renderer/Texture.h>

#include <vulkan/vulkan.h>

namespace Hazel {

	class VulkanBaseTexture
	{
	public:
		virtual VkImage GetImage() const = 0;
		virtual VkImageView GetImageView() const = 0;
		virtual VkDeviceMemory GetMemory() const = 0;
		virtual VkDescriptorImageInfo GetDescriptorInfo() const = 0;
	};

	class VulkanTexture2D : public Texture2D, public VulkanBaseTexture
	{
	public:
		VulkanTexture2D(
			TextureFormat format,
			uint32_t width,
			uint32_t height,
			void* data,
			TextureWrap wrap = TextureWrap::Clamp
		);

		VulkanTexture2D(const std::string& path, bool srgb = false);

		virtual ~VulkanTexture2D();

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

		virtual RendererId GetRendererId() const override { return 0; }

		VkImage GetImage() const override { return m_Image; }
		VkImageView GetImageView() const override { return m_ImageView; }
		VkDeviceMemory GetMemory() const override { return m_Memory; }
		VkDescriptorImageInfo GetDescriptorInfo() const override { return m_DescriptorInfo; }

	public:
		bool operator==(const Texture& other) const override
		{
			return m_Image == ((VulkanTexture2D&)other).GetImage();
		}

	private:
		void Create(void* data, bool srgb);

	private:
		std::string m_FilePath;

		uint32_t m_Width, m_Height;

		TextureFormat m_Format;
		TextureWrap m_Wrap = TextureWrap::Clamp;

		bool m_IsHDR = false;

		bool m_Locked = false;
		bool m_Loaded = false;

		Buffer m_ImageData;

		VkImage m_Image;
		VkImageView m_ImageView;
		VkDeviceMemory m_Memory;
		VkDescriptorImageInfo m_DescriptorInfo;
	};

	class VulkanTextureCube : public TextureCube, public VulkanBaseTexture
	{
	public:
		VulkanTextureCube(TextureFormat format, uint32_t width, uint32_t height);
		VulkanTextureCube(const std::string& path);
		virtual ~VulkanTextureCube();

		virtual void Bind(uint32_t slot = 0) const;

		virtual void SetData(void* data, uint32_t size) override;

		virtual TextureFormat GetFormat() const { return m_Format; }
		virtual uint32_t GetWidth() const { return m_Width; }
		virtual uint32_t GetHeight() const { return m_Height; }
		// This function currently returns the expected number of mips based on image size,
		// not present mips in data
		virtual uint32_t GetMipLevelCount() const override;

		virtual const std::string& GetPath() const override { return m_FilePath; }

		virtual RendererId GetRendererId() const override { return 0; }

		VkImage GetImage() const override { return m_Image; }
		VkImageView GetImageView() const override { return m_ImageView; }
		VkDeviceMemory GetMemory() const override { return m_Memory; }
		VkDescriptorImageInfo GetDescriptorInfo() const override { return m_DescriptorInfo; }

	public:
		virtual bool operator==(const Texture& other) const override
		{
			return m_Image == ((VulkanTextureCube&)other).GetImage();
		}

	private:
		void Create(std::array<unsigned char*, 6> faces);

	private:
		std::string m_FilePath;

		TextureFormat m_Format;
		uint32_t m_Width, m_Height;
		uint32_t m_FaceWidth, m_FaceHeight;

		unsigned char* m_ImageData;

		VkImage m_Image;
		VkImageView m_ImageView;
		VkDeviceMemory m_Memory;
		VkDescriptorImageInfo m_DescriptorInfo;
		VkSampler m_Sampler;
	};

}