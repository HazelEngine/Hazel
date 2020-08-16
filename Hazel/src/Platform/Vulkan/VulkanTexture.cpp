#include "hzpch.h"
#include "VulkanTexture.h"

#include <Hazel/Renderer/Renderer.h>

#include <stb_image.h>

#include "VulkanContext.h"
#include "VulkanUtils.h"

namespace Hazel {

	static VkFormat HZToVKTextureFormat(TextureFormat format)
	{
		switch (format)
		{
		case Hazel::TextureFormat::RGB:				return VK_FORMAT_R8G8B8_UNORM;
		case Hazel::TextureFormat::RGBA:			return VK_FORMAT_R8G8B8A8_UNORM;
		case Hazel::TextureFormat::Float16:			return VK_FORMAT_R16G16B16A16_SFLOAT;
		}

		HZ_CORE_ASSERT(false, "Unknown texture format!")
		return VK_FORMAT_UNDEFINED;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// Texture2D
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	VulkanTexture2D::VulkanTexture2D(
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

	VulkanTexture2D::VulkanTexture2D(const std::string& path, bool srgb)
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

	VulkanTexture2D::~VulkanTexture2D()
	{

	}

	void VulkanTexture2D::Bind(uint32_t slot) const
	{

	}

	void VulkanTexture2D::SetData(void* data, uint32_t size)
	{
		// TODO: Implement
	}

	void VulkanTexture2D::Create(void* data, bool srgb)
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& vk_Device = vk_Context->GetDevice();
		const uint32_t queueFamilyIndices[1] = { vk_Device->GetQueueFamilyIndices().GraphicsFamily.value() };

		VkFormat format = HZToVKTextureFormat(m_Format);

		// Create the image

		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.pQueueFamilyIndices = queueFamilyIndices;
		imageInfo.queueFamilyIndexCount = 1;
		imageInfo.mipLevels = 1;
		imageInfo.format = format;
		imageInfo.arrayLayers = 1;
		imageInfo.extent.width = m_Width;
		imageInfo.extent.height = m_Height;
		imageInfo.extent.depth = 1;
		imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VK_CHECK_RESULT(vkCreateImage(
			vk_Device->GetLogicalDevice(), 
			&imageInfo, 
			nullptr,
			&m_Image
		));

		// Allocate memory for image

		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(vk_Device->GetLogicalDevice(), m_Image, &memReqs);

		vk_Context->GetMemoryAllocator()->Allocate(
			memReqs,
			&m_Memory,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		vkBindImageMemory(vk_Device->GetLogicalDevice(), m_Image, m_Memory, 0);

		// Create the staging buffer

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pQueueFamilyIndices = queueFamilyIndices;
		bufferInfo.queueFamilyIndexCount = 1;
		bufferInfo.size = memReqs.size;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		VkBuffer stagingBuffer;
		VK_CHECK_RESULT(vkCreateBuffer(
			vk_Device->GetLogicalDevice(),
			&bufferInfo,
			nullptr,
			&stagingBuffer
		));

		VkMemoryRequirements bufferMemReqs;
		vkGetBufferMemoryRequirements(vk_Device->GetLogicalDevice(), stagingBuffer, &bufferMemReqs);

		VkDeviceMemory bufferMemory;
		vk_Context->GetMemoryAllocator()->Allocate(
			bufferMemReqs,
			&bufferMemory,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		);

		vkBindBufferMemory(vk_Device->GetLogicalDevice(), stagingBuffer, bufferMemory, 0);

		// Map the buffer memory and copy image data to it

		void* mapped = nullptr;
		vkMapMemory(vk_Device->GetLogicalDevice(), bufferMemory, 0, VK_WHOLE_SIZE, 0, &mapped);

		memcpy(mapped, data, bufferInfo.size);

		// This is needed as memory isn't HOST COHERENT
		VkMappedMemoryRange mappedMemoryRange = {};
		mappedMemoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedMemoryRange.memory = bufferMemory;
		mappedMemoryRange.offset = 0;
		mappedMemoryRange.size = VK_WHOLE_SIZE;

		VK_CHECK_RESULT(vkFlushMappedMemoryRanges(
			vk_Device->GetLogicalDevice(),
			1,
			&mappedMemoryRange
		));

		// Unmap memory

		vkUnmapMemory(vk_Device->GetLogicalDevice(), bufferMemory);

		// Create the buffer to image copy structure

		VkBufferImageCopy bufferImageCopy = {};
		bufferImageCopy.imageExtent.width = m_Width;
		bufferImageCopy.imageExtent.height = m_Height;
		bufferImageCopy.imageExtent.depth = 1;
		bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferImageCopy.imageSubresource.mipLevel = 0;
		bufferImageCopy.imageSubresource.layerCount = 1;

		// Buffer copies have to be submitted to a queue, so we need a command buffer for them
		// Note: Some devices offer a dedicated transfer queue (with only the transfer bit set)
		// that may be faster when doing lots of copies
		VkCommandBuffer copyCmd = vk_Context->GetCommandBuffer(true);

		// Create the image barrier to change the layout of image

		VkImageMemoryBarrier imageBarrier = {};
		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier.srcAccessMask = 0;
		imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier.image = m_Image;
		imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBarrier.subresourceRange.layerCount = 1;
		imageBarrier.subresourceRange.levelCount = 1;

		// Barrier to change image stage to TRANSFER
		vkCmdPipelineBarrier(
			copyCmd,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&imageBarrier
		);

		// Copy the image data in the buffer to the image
		vkCmdCopyBufferToImage(
			copyCmd,
			stagingBuffer,
			m_Image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&bufferImageCopy
		);

		imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// Barrier to change image stage to FRAGMENT SHADER
		vkCmdPipelineBarrier(
			copyCmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&imageBarrier
		);

		// Flushing the command buffer will also submit it to the queue and uses a
		// fence to ensure that all commands have been executed before returning
		vk_Context->FlushCommandBuffer(copyCmd);

		// Create the ImageView

		VkImageViewCreateInfo imageViewInfo = {};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.format = imageInfo.format;
		imageViewInfo.image = m_Image;
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.layerCount = 1;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

		VK_CHECK_RESULT(vkCreateImageView(
			vk_Device->GetLogicalDevice(),
			&imageViewInfo,
			nullptr,
			&m_ImageView
		));

		// Create a Descriptor

		m_DescriptorInfo = {};
		m_DescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorInfo.imageView = m_ImageView;
		//m_DescriptorInfo.sampler // TODO: Set the sampler here
	}

	void VulkanTexture2D::Lock()
	{
		m_Locked = true;
	}

	void VulkanTexture2D::Unlock()
	{
		m_Locked = false;
		// TODO: Upload updated buffer data
		// glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, HazelToOpenGLTextureFormat(m_Format), GL_UNSIGNED_BYTE, m_ImageData.Data);
	}

	void VulkanTexture2D::Resize(uint32_t width, uint32_t height)
	{
		HZ_CORE_ASSERT(m_Locked, "Texture must be locked!")

		uint32_t bpp = Texture::GetBPP(m_Format);
		m_ImageData.Allocate(width * height * bpp);

#if HZ_DEBUG
		m_ImageData.ZeroInitialize();
#endif
	}

	Buffer VulkanTexture2D::GetWriteableBuffer()
	{
		HZ_CORE_ASSERT(m_Locked, "Texture must be locked!")
		return m_ImageData;
	}

	uint32_t VulkanTexture2D::GetMipLevelCount() const
	{
		return Texture::CalculateMipMapCount(m_Width, m_Height);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////
	// TextureCube
	///////////////////////////////////////////////////////////////////////////////////////////////////////

	VulkanTextureCube::VulkanTextureCube(TextureFormat format, uint32_t width, uint32_t height)
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

	VulkanTextureCube::VulkanTextureCube(const std::string& path)
		: m_FilePath(path)
	{
		HZ_PROFILE_FUNCTION()

		int width, height, channels;
		stbi_set_flip_vertically_on_load(false);
		m_ImageData = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb);
		HZ_CORE_ASSERT(m_ImageData, "Failed to load image!")

		m_Width = width;
		m_Height = height;
		m_Format = TextureFormat::RGBA;

		m_FaceWidth = m_Width / 4;
		m_FaceHeight = m_Height / 3;

		uint32_t faceSize = m_FaceWidth * m_FaceHeight * 4; // 4 BPP
		HZ_CORE_ASSERT(m_FaceWidth == m_FaceHeight, "Non-square faces!")

		std::array<unsigned char*, 6> faces;
		for (size_t i = 0; i < faces.size(); i++) {
			faces[i] = new unsigned char[faceSize];
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

		// Process +X, +Z, -X, -Z
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
					size_t destCoord = (x + y * m_FaceWidth) * 4;
					faces[faceIndex][destCoord + 0] = m_ImageData[srcCoord + 0];
					faces[faceIndex][destCoord + 1] = m_ImageData[srcCoord + 1];
					faces[faceIndex][destCoord + 2] = m_ImageData[srcCoord + 2];
					faces[faceIndex][destCoord + 3] = 0xFF;
				}
			}

			processedFaces++;
		}

		// Process +Y and -Y
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
					size_t destCoord = (x + y * m_FaceWidth) * 4;
					faces[faceIndex][destCoord + 0] = m_ImageData[srcCoord + 0];
					faces[faceIndex][destCoord + 1] = m_ImageData[srcCoord + 1];
					faces[faceIndex][destCoord + 2] = m_ImageData[srcCoord + 2];
					faces[faceIndex][destCoord + 3] = 0xFF;
				}
			}

			processedFaces++;
		}

		// Create the cubemap
		Create(faces);

		// And free the raw image data
		stbi_image_free(m_ImageData);
	}

	VulkanTextureCube::~VulkanTextureCube()
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& vk_Device = vk_Context->GetDevice();

		vkDestroyImageView(vk_Device->GetLogicalDevice(), m_ImageView, nullptr);
		vkDestroyImage(vk_Device->GetLogicalDevice(), m_Image, nullptr);
		vkFreeMemory(vk_Device->GetLogicalDevice(), m_Memory, nullptr);
	}

	void VulkanTextureCube::Bind(uint32_t slot) const
	{
	}

	void VulkanTextureCube::SetData(void* data, uint32_t size)
	{
		// TODO: Implement
	}

	uint32_t VulkanTextureCube::GetMipLevelCount() const
	{
		return Texture::CalculateMipMapCount(m_Width, m_Height);
	}

	void VulkanTextureCube::Create(std::array<unsigned char*, 6> faces)
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& vk_Device = vk_Context->GetDevice();
		const uint32_t queueFamilyIndices[1] = { vk_Device->GetQueueFamilyIndices().GraphicsFamily.value() };

		VkFormat format = HZToVKTextureFormat(m_Format);
		uint32_t faceSize = m_FaceWidth * m_FaceHeight * Texture::GetBPP(m_Format);

		// Create the image

		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.pQueueFamilyIndices = queueFamilyIndices;
		imageInfo.queueFamilyIndexCount = 1;
		imageInfo.mipLevels = 1;
		imageInfo.format = format;
		imageInfo.arrayLayers = faces.size();
		imageInfo.extent.width = m_FaceWidth;
		imageInfo.extent.height = m_FaceHeight;
		imageInfo.extent.depth = 1;
		imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: 16
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		VK_CHECK_RESULT(vkCreateImage(
			vk_Device->GetLogicalDevice(),
			&imageInfo,
			nullptr,
			&m_Image
		));

		// Allocate memory for image

		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(vk_Device->GetLogicalDevice(), m_Image, &memReqs);

		vk_Context->GetMemoryAllocator()->Allocate(
			memReqs,
			&m_Memory,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		vkBindImageMemory(vk_Device->GetLogicalDevice(), m_Image, m_Memory, 0);

		// Create the staging buffer

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.pQueueFamilyIndices = queueFamilyIndices;
		bufferInfo.queueFamilyIndexCount = 1;
		bufferInfo.size = memReqs.size;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		VkBuffer stagingBuffer;
		VK_CHECK_RESULT(vkCreateBuffer(
			vk_Device->GetLogicalDevice(),
			&bufferInfo,
			nullptr,
			&stagingBuffer
		));

		VkMemoryRequirements bufferMemReqs;
		vkGetBufferMemoryRequirements(vk_Device->GetLogicalDevice(), stagingBuffer, &bufferMemReqs);

		VkDeviceMemory bufferMemory;
		vk_Context->GetMemoryAllocator()->Allocate(
			bufferMemReqs,
			&bufferMemory,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		vkBindBufferMemory(vk_Device->GetLogicalDevice(), stagingBuffer, bufferMemory, 0);

		// Map the buffer memory and copy image data to it

		void* mapped = nullptr;
		vkMapMemory(vk_Device->GetLogicalDevice(), bufferMemory, 0, VK_WHOLE_SIZE, 0, &mapped);

		for (size_t i = 0; i < faces.size(); i++)
		{
			uint32_t offset = (faceSize * i);
			memcpy((char*)mapped + offset, faces[i], faceSize);
		}

		// Unmap memory

		vkUnmapMemory(vk_Device->GetLogicalDevice(), bufferMemory);

		// Create the buffer to image copy structure

		VkBufferImageCopy bufferImageCopy = {};
		bufferImageCopy.imageExtent.width = m_FaceWidth;
		bufferImageCopy.imageExtent.height = m_FaceHeight;
		bufferImageCopy.imageExtent.depth = 1;
		bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferImageCopy.imageSubresource.mipLevel = 0;
		bufferImageCopy.imageSubresource.layerCount = faces.size();

		// Buffer copies have to be submitted to a queue, so we need a command buffer for them
		// Note: Some devices offer a dedicated transfer queue (with only the transfer bit set)
		// that may be faster when doing lots of copies
		VkCommandBuffer copyCmd = vk_Context->GetCommandBuffer(true);

		// Create the image barrier to change the layout of image

		VkImageMemoryBarrier imageBarrier = {};
		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier.srcAccessMask = 0;
		imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier.image = m_Image;
		imageBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageBarrier.subresourceRange.layerCount = faces.size();
		imageBarrier.subresourceRange.levelCount = 1;

		// Barrier to change image stage to TRANSFER
		vkCmdPipelineBarrier(
			copyCmd,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&imageBarrier
		);

		// Copy the image data in the buffer to the image
		vkCmdCopyBufferToImage(
			copyCmd,
			stagingBuffer,
			m_Image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&bufferImageCopy
		);

		imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// Barrier to change image stage to FRAGMENT SHADER
		vkCmdPipelineBarrier(
			copyCmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0,
			nullptr,
			0,
			nullptr,
			1,
			&imageBarrier
		);

		// Flushing the command buffer will also submit it to the queue and uses a
		// fence to ensure that all commands have been executed before returning
		vk_Context->FlushCommandBuffer(copyCmd);

		// Create the ImageView

		VkImageViewCreateInfo imageViewInfo = {};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.format = imageInfo.format;
		imageViewInfo.image = m_Image;
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.layerCount = faces.size();
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;

		VK_CHECK_RESULT(vkCreateImageView(
			vk_Device->GetLogicalDevice(),
			&imageViewInfo,
			nullptr,
			&m_ImageView
		));

		// Create Sampler

		VkSamplerCreateInfo samplerInfo = {};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		//samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.magFilter = VK_FILTER_NEAREST; // VK_FILTER_LINEAR when MipMap
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		//samplerInfo.anisotropyEnable = true; // TODO: Get from Capabilities!
		//samplerInfo.maxAnisotropy = 16.0f;

		VK_CHECK_RESULT(vkCreateSampler(
			vk_Device->GetLogicalDevice(),
			&samplerInfo,
			nullptr,
			&m_Sampler
		));

		// Create a Descriptor

		m_DescriptorInfo = {};
		m_DescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_DescriptorInfo.imageView = m_ImageView;
		m_DescriptorInfo.sampler = m_Sampler;

		// Now, we can delete this data
		for (size_t i = 0; i < faces.size(); i++) {
			delete[] faces[i];
		}
	}

}