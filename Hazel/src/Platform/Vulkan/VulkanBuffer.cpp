#include "hzpch.h"
#include "VulkanBuffer.h"

#include <Hazel/Renderer/Renderer.h>

#include "VulkanContext.h"
#include "VulkanUtils.h"

namespace Hazel {
	
	VulkanVertexBuffer::VulkanVertexBuffer(uint32_t size)
		: m_Size(size)
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& device = vk_Context->GetDevice();
		auto& allocator = vk_Context->GetMemoryAllocator();

		VkMemoryRequirements memReqs;
		VkBufferCreateInfo vertexBufferInfo = {};
		vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertexBufferInfo.size = size;
		// Buffer is used as the copy source
		vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		// Create a host-visible buffer to copy the vertex data to (staging buffer)
		VK_CHECK_RESULT(vkCreateBuffer(device->GetLogicalDevice(), &vertexBufferInfo, nullptr, &m_StagingBuffer));
		vkGetBufferMemoryRequirements(device->GetLogicalDevice(), m_StagingBuffer, &memReqs);
		allocator->Allocate(memReqs, &m_StagingMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VK_CHECK_RESULT(vkBindBufferMemory(device->GetLogicalDevice(), m_StagingBuffer, m_StagingMemory, 0));

		// Create a device local buffer to which the (host local) vertex data will be copied and which will be used for rendering
		vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VK_CHECK_RESULT(vkCreateBuffer(device->GetLogicalDevice(), &vertexBufferInfo, nullptr, &m_DeviceBuffer));
		vkGetBufferMemoryRequirements(device->GetLogicalDevice(), m_DeviceBuffer, &memReqs);
		allocator->Allocate(memReqs, &m_DeviceMemory);
		VK_CHECK_RESULT(vkBindBufferMemory(device->GetLogicalDevice(), m_DeviceBuffer, m_DeviceMemory, 0));
	}

	VulkanVertexBuffer::VulkanVertexBuffer(void* data, uint32_t size)
		: m_Size(size)
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& device = vk_Context->GetDevice();
		auto& allocator = vk_Context->GetMemoryAllocator();
		
		VkMemoryRequirements memReqs;
		void* dataBuffer;
		
		VkBufferCreateInfo vertexBufferInfo = {};
		vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertexBufferInfo.size = size;
		// Buffer is used as the copy source
		vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		// Create a host-visible buffer to copy the vertex data to (staging buffer)
		VK_CHECK_RESULT(vkCreateBuffer(device->GetLogicalDevice(), &vertexBufferInfo, nullptr, &m_StagingBuffer));
		vkGetBufferMemoryRequirements(device->GetLogicalDevice(), m_StagingBuffer, &memReqs);
		allocator->Allocate(memReqs, &m_StagingMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VK_CHECK_RESULT(vkBindBufferMemory(device->GetLogicalDevice(), m_StagingBuffer, m_StagingMemory, 0));
		// Map and copy
		VK_CHECK_RESULT(vkMapMemory(device->GetLogicalDevice(), m_StagingMemory, 0, memReqs.size, 0, &dataBuffer));
		memcpy(dataBuffer, data, size);
		vkUnmapMemory(device->GetLogicalDevice(), m_StagingMemory);
		
		// Create a device local buffer to which the (host local) vertex data will be copied and which will be used for rendering
		vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		VK_CHECK_RESULT(vkCreateBuffer(device->GetLogicalDevice(), &vertexBufferInfo, nullptr, &m_DeviceBuffer));
		vkGetBufferMemoryRequirements(device->GetLogicalDevice(), m_DeviceBuffer, &memReqs);
		allocator->Allocate(memReqs, &m_DeviceMemory);
		VK_CHECK_RESULT(vkBindBufferMemory(device->GetLogicalDevice(), m_DeviceBuffer, m_DeviceMemory, 0));

		// Buffer copies have to be submitted to a queue, so we need a command buffer for them
		// Note: Some devices offer a dedicated transfer queue (with only the transfer bit set) that may be faster when doing lots of copies
		VkCommandBuffer copyCmd = vk_Context->GetCommandBuffer(true);

		// Put buffer region copies into command buffer
		VkBufferCopy copyRegion = {};
		copyRegion.size = size;
		vkCmdCopyBuffer(copyCmd, m_StagingBuffer, m_DeviceBuffer, 1, &copyRegion);

		// Flushing the command buffer will also submit it to the queue and uses a fence to ensure that all commands have been executed before returning
		vk_Context->FlushCommandBuffer(copyCmd);

		// Destroy staging buffers
		// Note: Staging buffer must not be deleted before the copies have been submitted and executed
		vkDestroyBuffer(device->GetLogicalDevice(), m_StagingBuffer, nullptr);
		vkFreeMemory(device->GetLogicalDevice(), m_StagingMemory, nullptr);
	}

	void* VulkanVertexBuffer::Map()
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& device = vk_Context->GetDevice();

		void* data;
		VK_CHECK_RESULT(vkMapMemory(device->GetLogicalDevice(), m_StagingMemory, 0, m_Size, 0, &data));
		return data;
	}

	void VulkanVertexBuffer::Unmap(uint32_t size)
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& device = vk_Context->GetDevice();

		vkUnmapMemory(device->GetLogicalDevice(), m_StagingMemory);

		// Copy data from staging to device
		VkCommandBuffer copyCmd = vk_Context->GetCommandBuffer(true);
		VkBufferCopy copyRegion = {};
		copyRegion.size = m_Size;
		vkCmdCopyBuffer(copyCmd, m_StagingBuffer, m_DeviceBuffer, 1, &copyRegion);
		vk_Context->FlushCommandBuffer(copyCmd);
	}

	VulkanIndexBuffer::VulkanIndexBuffer(uint32_t* data, uint32_t count)
		: m_Count(count)
	{
		uint32_t size = count * sizeof(uint32_t);

		bool useStagingBuffers = true;

		VkMemoryRequirements memReqs;
		void* dataBuffer;

		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& device = vk_Context->GetDevice();
		auto& allocator = vk_Context->GetMemoryAllocator();

		if (useStagingBuffers)
		{
			struct StagingBuffer
			{
				VkDeviceMemory memory;
				VkBuffer buffer;
			};

			struct {
				StagingBuffer indices;
			} stagingBuffers;

			// Index Buffer
			VkBufferCreateInfo indexBufferInfo = {};
			indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			indexBufferInfo.size = size;
			indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			// Copy index data to a buffer visible to the host (staging buffer)
			VK_CHECK_RESULT(vkCreateBuffer(device->GetLogicalDevice(), &indexBufferInfo, nullptr, &stagingBuffers.indices.buffer));
			vkGetBufferMemoryRequirements(device->GetLogicalDevice(), stagingBuffers.indices.buffer, &memReqs);
			allocator->Allocate(memReqs, &stagingBuffers.indices.memory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			VK_CHECK_RESULT(vkMapMemory(device->GetLogicalDevice(), stagingBuffers.indices.memory, 0, size, 0, &dataBuffer));
			memcpy(dataBuffer, data, size);
			vkUnmapMemory(device->GetLogicalDevice(), stagingBuffers.indices.memory);
			VK_CHECK_RESULT(vkBindBufferMemory(device->GetLogicalDevice(), stagingBuffers.indices.buffer, stagingBuffers.indices.memory, 0));

			// Create destination buffer with device only visibility
			indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			VK_CHECK_RESULT(vkCreateBuffer(device->GetLogicalDevice(), &indexBufferInfo, nullptr, &m_DeviceBuffer));
			vkGetBufferMemoryRequirements(device->GetLogicalDevice(), m_DeviceBuffer, &memReqs);
			allocator->Allocate(memReqs, &m_DeviceMemory);
			VK_CHECK_RESULT(vkBindBufferMemory(device->GetLogicalDevice(), m_DeviceBuffer, m_DeviceMemory, 0));

			// Buffer copies have to be submitted to a queue, so we need a command buffer for them
			// Note: Some devices offer a dedicated transfer queue (with only the transfer bit set) that may be faster when doing lots of copies
			VkCommandBuffer copyCmd = vk_Context->GetCommandBuffer(true);

			// Put buffer region copies into command buffer
			VkBufferCopy copyRegion = {};
			copyRegion.size = size;
			vkCmdCopyBuffer(copyCmd, stagingBuffers.indices.buffer, m_DeviceBuffer, 1, &copyRegion);

			// Flushing the command buffer will also submit it to the queue and uses a fence to ensure that all commands have been executed before returning
			vk_Context->FlushCommandBuffer(copyCmd);

			// Destroy staging buffers
			// Note: Staging buffer must not be deleted before the copies have been submitted and executed
			vkDestroyBuffer(device->GetLogicalDevice(), stagingBuffers.indices.buffer, nullptr);
			vkFreeMemory(device->GetLogicalDevice(), stagingBuffers.indices.memory, nullptr);
		}
		else
		{
			// Don't use staging
			// Create host visible buffers only and use these for rendering. This is not advised and will usually result in lower rendering performance
			VkBufferCreateInfo indexBufferInfo = {};
			indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			indexBufferInfo.size = size;
			indexBufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

			// Copy index data to a buffer visible to the host
			VK_CHECK_RESULT(vkCreateBuffer(device->GetLogicalDevice(), &indexBufferInfo, nullptr, &m_DeviceBuffer));
			vkGetBufferMemoryRequirements(device->GetLogicalDevice(), m_DeviceBuffer, &memReqs);
			allocator->Allocate(memReqs, &m_DeviceMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			VK_CHECK_RESULT(vkMapMemory(device->GetLogicalDevice(), m_DeviceMemory, 0, size, 0, &dataBuffer));
			memcpy(dataBuffer, data, size);
			vkUnmapMemory(device->GetLogicalDevice(), m_DeviceMemory);
			VK_CHECK_RESULT(vkBindBufferMemory(device->GetLogicalDevice(), m_DeviceBuffer, m_DeviceMemory, 0));
		}
	}

	VulkanUniformBuffer::VulkanUniformBuffer(uint32_t size)
		: m_Size(size)
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& vk_Device = vk_Context->GetDevice();
		auto& vk_Allocator = vk_Context->GetMemoryAllocator();

		// Create buffer

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

		VK_CHECK_RESULT(vkCreateBuffer(
			vk_Device->GetLogicalDevice(),
			&bufferInfo,
			nullptr,
			&m_DeviceBuffer
		));

		// Allocate memory

		VkMemoryRequirements memReqs;
		vkGetBufferMemoryRequirements(vk_Device->GetLogicalDevice(), m_DeviceBuffer, &memReqs);

		vk_Allocator->Allocate(
			memReqs,
			&m_DeviceMemory, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		vkBindBufferMemory(vk_Device->GetLogicalDevice(), m_DeviceBuffer, m_DeviceMemory, 0);

		// Create the buffer info for Descriptor binding

		m_DescriptorInfo = {};
		m_DescriptorInfo.buffer = m_DeviceBuffer;
		m_DescriptorInfo.offset = 0;
		m_DescriptorInfo.range = VK_WHOLE_SIZE;
	}

	VulkanUniformBuffer::VulkanUniformBuffer(void* data, uint32_t size)
		: m_Size(size)
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& vk_Device = vk_Context->GetDevice();
		auto& vk_Allocator = vk_Context->GetMemoryAllocator();

		// Create buffer

		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

		VK_CHECK_RESULT(vkCreateBuffer(
			vk_Device->GetLogicalDevice(),
			&bufferInfo,
			nullptr,
			&m_DeviceBuffer
		));

		// Allocate memory

		VkMemoryRequirements memReqs;
		vkGetBufferMemoryRequirements(vk_Device->GetLogicalDevice(), m_DeviceBuffer, &memReqs);

		vk_Allocator->Allocate(
			memReqs,
			&m_DeviceMemory, 
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		);

		vkBindBufferMemory(vk_Device->GetLogicalDevice(), m_DeviceBuffer, m_DeviceMemory, 0);

		// Map and copy data
		void* mapped = nullptr;
		VK_CHECK_RESULT(vkMapMemory(
			vk_Device->GetLogicalDevice(),
			m_DeviceMemory,
			0,
			VK_WHOLE_SIZE,
			0,
			&mapped
		));

		memcpy(mapped, data, size);

		vkUnmapMemory(vk_Device->GetLogicalDevice(), m_DeviceMemory);

		// Create the buffer info for Descriptor binding

		m_DescriptorInfo = {};
		m_DescriptorInfo.buffer = m_DeviceBuffer;
		m_DescriptorInfo.offset = 0;
		m_DescriptorInfo.range = VK_WHOLE_SIZE;
	}

	void* VulkanUniformBuffer::Map()
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& vk_Device = vk_Context->GetDevice();

		void* data;
		VK_CHECK_RESULT(vkMapMemory(
			vk_Device->GetLogicalDevice(),
			m_DeviceMemory,
			0,
			VK_WHOLE_SIZE,
			0,
			&data
		));
		
		return data;
	}

	void VulkanUniformBuffer::Unmap(uint32_t size)
	{
		Unmap(0, size);
	}

	void VulkanUniformBuffer::Unmap(uint32_t offset, uint32_t size)
	{
		VulkanContext* vk_Context = dynamic_cast<VulkanContext*>(Renderer::GetContext());
		auto& vk_Device = vk_Context->GetDevice();

		vkUnmapMemory(vk_Device->GetLogicalDevice(), m_DeviceMemory);
	}

}
