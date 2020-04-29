#pragma once

#include <Hazel/Renderer/Texture.h>

#include <vulkan/vulkan.h>

namespace Hazel {

	class HAZEL_API VulkanTexture2D : public Texture2D
	{
	public:
		VulkanTexture2D(void* data, uint32_t width, uint32_t height, uint32_t channels);
		virtual ~VulkanTexture2D();

		virtual void Bind(uint32_t slot = 0) const override;

		virtual void SetData(void* data, uint32_t size) override;

		virtual uint32_t GetWidth() const override { return m_Width; }
		virtual uint32_t GetHeight() const override { return m_Height; }

		VkImage GetImage() const { return m_Image; }
		VkImageView GetImageView() const { return m_ImageView; }
		VkDeviceMemory GetMemory() const { return m_Memory; }
		VkDescriptorImageInfo GetDescriptorInfo() const { return m_DescriptorInfo; }

	public:
		bool operator==(const Texture& other) const override
		{
			// TODO: Verify if are truly equal
			return true;
		}

	private:
		void Create(void* data, uint32_t width, uint32_t height, uint32_t channels);

	private:
	private:
		std::string m_Path;
		uint32_t m_Width, m_Height;
		VkFormat m_Format;

		VkImage m_Image;
		VkImageView m_ImageView;
		VkDeviceMemory m_Memory;
		VkDescriptorImageInfo m_DescriptorInfo;
	};

}