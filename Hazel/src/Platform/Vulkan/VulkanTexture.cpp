#include "hzpch.h"
#include "VulkanTexture.h"

#include <Hazel/Renderer/Renderer.h>

#include "VulkanContext.h"
#include "VulkanUtils.h"

namespace Hazel {

	VulkanTexture2D::VulkanTexture2D(
		void* data,
		uint32_t width,
		uint32_t height,
		uint32_t channels
	) : m_Path(""), m_Width(width), m_Height(height)
	{
		Create(data, width, height, channels);
	}

	VulkanTexture2D::~VulkanTexture2D()
	{

	}

	void VulkanTexture2D::Bind(uint32_t slot) const
	{

	}

	void VulkanTexture2D::SetData(void* data, uint32_t size)
	{

	}

	void VulkanTexture2D::Create(void* data, uint32_t width, uint32_t height, uint32_t channels)
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& vk_Device = vk_Context->GetDevice();
		const uint32_t queueFamilyIndices[1] = { vk_Device->GetQueueFamilyIndices().GraphicsFamily.value() };

		if (channels == 3)
		{
			m_Format = VK_FORMAT_R8G8B8_UNORM;
		}
		else if (channels == 4)
		{
			m_Format = VK_FORMAT_R8G8B8A8_UNORM;
		}

		// Create the image

		VkImageCreateInfo imageInfo = {};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.pQueueFamilyIndices = queueFamilyIndices;
		imageInfo.queueFamilyIndexCount = 1;
		imageInfo.mipLevels = 1;
		imageInfo.format = m_Format;
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
		bufferImageCopy.imageExtent.width = width;
		bufferImageCopy.imageExtent.height = height;
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

		// Now, we can delete this data
		//free(data);

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

}